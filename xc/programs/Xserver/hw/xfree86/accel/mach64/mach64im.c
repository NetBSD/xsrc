/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/mach64/mach64im.c,v 3.5.2.2 1999/05/29 08:25:53 dawes Exp $ */
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
 */
/* $XConsortium: mach64im.c /main/4 1996/05/13 16:36:12 kaleb $ */

#include "X.h"
#include "misc.h"
#ifndef FBDEV_SERVER
#include "xf86.h"
#include "xf86_HWlib.h"
#endif
#include "mach64.h"
#include "mach64im.h"

#if (BITMAP_BIT_ORDER == MSBFirst)

static unsigned char bittab[256] = {
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
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

union intbytes {
    unsigned int integer;
    unsigned char bytes[4];
};

static inline unsigned int bit_reverse32(unsigned int bits)
{
    union intbytes c;

    c.bytes[0] = bittab[((union intbytes)bits).bytes[3]];
    c.bytes[1] = bittab[((union intbytes)bits).bytes[2]];
    c.bytes[2] = bittab[((union intbytes)bits).bytes[1]];
    c.bytes[3] = bittab[((union intbytes)bits).bytes[0]];
    return c.integer;
}

#define  byte_reverse16(bytes)		\
    ((((bytes) & 0xff000000) >> 8) |	\
     (((bytes) & 0x00ff0000) << 8) |	\
     (((bytes) & 0x0000ff00) >> 8) |	\
     (((bytes) & 0x000000ff) << 8))

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

#endif

void	(*mach64ImageReadFunc)();
void	(*mach64ImageWriteFunc)();

static void mach64ImageRead();
static void mach64ImageWrite();
static void mach64ImageWriteHW();

static unsigned long PMask;
static int BytesPerPixel;
static int screenStride;

void
mach64ImageInit()
{
    int i;

    PMask = (1UL << mach64InfoRec.depth) - 1;
    BytesPerPixel = mach64InfoRec.bitsPerPixel / 8;
    screenStride = mach64VirtX * BytesPerPixel;

    mach64ImageReadFunc = mach64ImageRead;
    mach64ImageWriteFunc = mach64ImageWrite;
}

static void
mach64ImageWrite(x, y, w, h, psrc, pwidth, px, py, alu, planemask)
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

    if (alu == MIX_DST)
	return;

    if ((alu != MIX_SRC) || ((planemask & PMask) != PMask)) {
	mach64ImageWriteHW(x, y, w, h, psrc, pwidth, px, py,
			   alu, planemask);
	return;
    }
	
    WaitIdleEmpty();

    psrc += pwidth * py + px * BytesPerPixel;
    curvm = (pointer)((unsigned char *)mach64VideoMem +
	    (x + y * mach64VirtX) * BytesPerPixel);

    byteCount = w * BytesPerPixel;
    while(h--) {
	MemToBus((void *)curvm, psrc, byteCount);
	curvm = (pointer)((unsigned char *)curvm + screenStride); 
	psrc += pwidth;
    }
}


static void
mach64ImageWriteHW(x, y, w, h, psrc, pwidth, px, py, alu, planemask)
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
#if (BITMAP_BIT_ORDER == MSBFirst)
    unsigned int reversed_pixmap[16];
    int i;
#endif

    WaitQueue(8);
    old_DP_PIX_WIDTH = regr(DP_PIX_WIDTH);
    regw(SC_LEFT_RIGHT, (((x+w-1) << 16) | (x & 0x0000ffff)));
    regw(SC_TOP_BOTTOM, (((y+h-1) << 16) | (y & 0x0000ffff)));
    regw(DP_WRITE_MASK, planemask);
    regw(DP_MIX, (alu << 16) | alu);
    regw(DP_SRC, (MONO_SRC_ONE | FRGD_SRC_HOST | BKGD_SRC_HOST));
    switch(mach64InfoRec.bitsPerPixel)
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
    }

    w = (w + 3) / 4; /* round up to int boundry */

    regw(DST_Y_X, ((x << 16) | (y & 0x0000ffff)));
    regw(DST_HEIGHT_WIDTH, (((w * 4) << 16) | (h & 0x0000ffff)));

    psrc += pwidth * py + px * BytesPerPixel;
    wordsPerLine = w * BytesPerPixel; 

#if (BITMAP_BIT_ORDER == MSBFirst)
    switch(mach64InfoRec.bitsPerPixel) {
      case 32:
	while (h--) {
	  count = wordsPerLine;
	  pword = (int *)psrc;

	  while (count >= 16) {
            WaitQueue(16);
            regw(HOST_DATAF, *pword++);
            regw(HOST_DATAE, *pword++);
            regw(HOST_DATAD, *pword++);
            regw(HOST_DATAC, *pword++);
            regw(HOST_DATAB, *pword++);
            regw(HOST_DATAA, *pword++);
            regw(HOST_DATA9, *pword++);
            regw(HOST_DATA8, *pword++);
            regw(HOST_DATA7, *pword++);
            regw(HOST_DATA6, *pword++);
            regw(HOST_DATA5, *pword++);
            regw(HOST_DATA4, *pword++);
            regw(HOST_DATA3, *pword++);
            regw(HOST_DATA2, *pword++);
            regw(HOST_DATA1, *pword++);
            regw(HOST_DATA0, *pword++);
	    count -= 16;
	  }
	  WaitQueue(count);
	  switch(count)	{
	    case 15: regw(HOST_DATAE, *pword++);
	    case 14: regw(HOST_DATAD, *pword++);
	    case 13: regw(HOST_DATAC, *pword++);
	    case 12: regw(HOST_DATAB, *pword++);
	    case 11: regw(HOST_DATAA, *pword++);
	    case 10: regw(HOST_DATA9, *pword++);
	    case  9: regw(HOST_DATA8, *pword++);
	    case  8: regw(HOST_DATA7, *pword++);
	    case  7: regw(HOST_DATA6, *pword++);
	    case  6: regw(HOST_DATA5, *pword++);
	    case  5: regw(HOST_DATA4, *pword++);
	    case  4: regw(HOST_DATA3, *pword++);
	    case  3: regw(HOST_DATA2, *pword++);
	    case  2: regw(HOST_DATA1, *pword++);
	    case  1: regw(HOST_DATA0, *pword);
	    default:
		break;
	  }
	  psrc += pwidth;
	}
	break;

      case 16:
	while (h--) {
	  count = wordsPerLine;
	  pword = (int *)psrc;

	  while (count >= 16) {
	    for(i = 15; i >= 0; i--) {
	      reversed_pixmap[i] = byte_reverse16(*pword);
	      ++pword;
 	    }
            WaitQueue(16);
            regwbe(HOST_DATAF, reversed_pixmap[0x0f]);
            regwbe(HOST_DATAE, reversed_pixmap[0x0e]);
            regwbe(HOST_DATAD, reversed_pixmap[0x0d]);
            regwbe(HOST_DATAC, reversed_pixmap[0x0c]);
            regwbe(HOST_DATAB, reversed_pixmap[0x0b]);
            regwbe(HOST_DATAA, reversed_pixmap[0x0a]);
            regwbe(HOST_DATA9, reversed_pixmap[9]);
            regwbe(HOST_DATA8, reversed_pixmap[8]);
            regwbe(HOST_DATA7, reversed_pixmap[7]);
            regwbe(HOST_DATA6, reversed_pixmap[6]);
            regwbe(HOST_DATA5, reversed_pixmap[5]);
            regwbe(HOST_DATA4, reversed_pixmap[4]);
            regwbe(HOST_DATA3, reversed_pixmap[3]);
            regwbe(HOST_DATA2, reversed_pixmap[2]);
            regwbe(HOST_DATA1, reversed_pixmap[1]);
            regwbe(HOST_DATA0, reversed_pixmap[0]);
	    count -= 16;
	  }
	  for(i = count - 1; i >= 0; i--) {
	    reversed_pixmap[i] = byte_reverse16(*pword);
	    ++pword;
	  }
	  WaitQueue(count);
	  switch(count)	{
	    case 15: regwbe(HOST_DATAE, reversed_pixmap[0x0e]);
	    case 14: regwbe(HOST_DATAD, reversed_pixmap[0x0d]);
	    case 13: regwbe(HOST_DATAC, reversed_pixmap[0x0c]);
	    case 12: regwbe(HOST_DATAB, reversed_pixmap[0x0b]);
	    case 11: regwbe(HOST_DATAA, reversed_pixmap[0x0a]);
	    case 10: regwbe(HOST_DATA9, reversed_pixmap[9]);
	    case  9: regwbe(HOST_DATA8, reversed_pixmap[8]);
	    case  8: regwbe(HOST_DATA7, reversed_pixmap[7]);
	    case  7: regwbe(HOST_DATA6, reversed_pixmap[6]);
	    case  6: regwbe(HOST_DATA5, reversed_pixmap[5]);
	    case  5: regwbe(HOST_DATA4, reversed_pixmap[4]);
	    case  4: regwbe(HOST_DATA3, reversed_pixmap[3]);
	    case  3: regwbe(HOST_DATA2, reversed_pixmap[2]);
	    case  2: regwbe(HOST_DATA1, reversed_pixmap[1]);
	    case  1: regwbe(HOST_DATA0, reversed_pixmap[0]);
	    default:
		break;
	  }
	  psrc += pwidth;
	}
	break;

      default:
	while (h--)  {
	  count = wordsPerLine;
	  pword = (int *)psrc;

	  while (count >= 16) {
            WaitQueue(16);
            regwbe(HOST_DATAF, *pword++);
            regwbe(HOST_DATAE, *pword++);
            regwbe(HOST_DATAD, *pword++);
            regwbe(HOST_DATAC, *pword++);
            regwbe(HOST_DATAB, *pword++);
            regwbe(HOST_DATAA, *pword++);
            regwbe(HOST_DATA9, *pword++);
            regwbe(HOST_DATA8, *pword++);
            regwbe(HOST_DATA7, *pword++);
            regwbe(HOST_DATA6, *pword++);
            regwbe(HOST_DATA5, *pword++);
            regwbe(HOST_DATA4, *pword++);
            regwbe(HOST_DATA3, *pword++);
            regwbe(HOST_DATA2, *pword++);
            regwbe(HOST_DATA1, *pword++);
            regwbe(HOST_DATA0, *pword++);
	    count -= 16;
	  }
	  WaitQueue(count);
	  switch(count)	{
	    case 15: regwbe(HOST_DATAE, *pword++);
	    case 14: regwbe(HOST_DATAD, *pword++);
	    case 13: regwbe(HOST_DATAC, *pword++);
	    case 12: regwbe(HOST_DATAB, *pword++);
	    case 11: regwbe(HOST_DATAA, *pword++);
	    case 10: regwbe(HOST_DATA9, *pword++);
	    case  9: regwbe(HOST_DATA8, *pword++);
	    case  8: regwbe(HOST_DATA7, *pword++);
	    case  7: regwbe(HOST_DATA6, *pword++);
	    case  6: regwbe(HOST_DATA5, *pword++);
	    case  5: regwbe(HOST_DATA4, *pword++);
	    case  4: regwbe(HOST_DATA3, *pword++);
	    case  3: regwbe(HOST_DATA2, *pword++);
	    case  2: regwbe(HOST_DATA1, *pword++);
	    case  1: regwbe(HOST_DATA0, *pword);
	    default:
		break;
	  }
	  psrc += pwidth;
	}
	break;
    }
#else
    while (h--) 
    {
	count = wordsPerLine;
	pword = (int *)psrc;

	while (count >= 16)
	{
            WaitQueue(16);
            regw(HOST_DATAF, *pword++);
            regw(HOST_DATAE, *pword++);
            regw(HOST_DATAD, *pword++);
            regw(HOST_DATAC, *pword++);
            regw(HOST_DATAB, *pword++);
            regw(HOST_DATAA, *pword++);
            regw(HOST_DATA9, *pword++);
            regw(HOST_DATA8, *pword++);
            regw(HOST_DATA7, *pword++);
            regw(HOST_DATA6, *pword++);
            regw(HOST_DATA5, *pword++);
            regw(HOST_DATA4, *pword++);
            regw(HOST_DATA3, *pword++);
            regw(HOST_DATA2, *pword++);
            regw(HOST_DATA1, *pword++);
            regw(HOST_DATA0, *pword++);
	    count -= 16;
	}

	WaitQueue(16);
	switch(count)
	{
	    case 15: regw(HOST_DATAE, *pword++);
	    case 14: regw(HOST_DATAD, *pword++);
	    case 13: regw(HOST_DATAC, *pword++);
	    case 12: regw(HOST_DATAB, *pword++);
	    case 11: regw(HOST_DATAA, *pword++);
	    case 10: regw(HOST_DATA9, *pword++);
	    case  9: regw(HOST_DATA8, *pword++);
	    case  8: regw(HOST_DATA7, *pword++);
	    case  7: regw(HOST_DATA6, *pword++);
	    case  6: regw(HOST_DATA5, *pword++);
	    case  5: regw(HOST_DATA4, *pword++);
	    case  4: regw(HOST_DATA3, *pword++);
	    case  3: regw(HOST_DATA2, *pword++);
	    case  2: regw(HOST_DATA1, *pword++);
	    case  1: regw(HOST_DATA0, *pword);
	    default:
		break;
	}
	psrc += pwidth;
    }
#endif

    WaitQueue(3);
    regw(SC_LEFT_RIGHT, ((mach64MaxX << 16) | 0 ));
    regw(SC_TOP_BOTTOM, ((mach64MaxY << 16) | 0 ));
    regw(DP_PIX_WIDTH, old_DP_PIX_WIDTH);

    WaitIdleEmpty();
}


static void
mach64ImageRead(x, y, w, h, psrc, pwidth, px, py, planemask)
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
        mach64ImageReadHW(x, y, w, h, psrc, pwidth, px, py, planemask);
	return;
#else
	ErrorF ("mach64ImageRead: unsupported planemask\n");
#endif
    }

    WaitIdleEmpty();

    psrc += pwidth * py + px * BytesPerPixel;
    curvm = (pointer)((unsigned char *)mach64VideoMem + x * BytesPerPixel);
    
    for (j = y; j < y+h; j++) {
	BusToMem(psrc, (void *)((unsigned char *)curvm + j * screenStride),
		 w * BytesPerPixel);
	psrc += pwidth;
    }
}

void
mach64ImageStippleFunc(x, y, w, h, psrc, pwidth, px, py, fgPixel, bgPixel, 
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

#if (BITMAP_BIT_ORDER == MSBFirst)
    unsigned int reversed_bitmap[16];
    int i;
#endif

    if (alu == MIX_DST || w <= 0 || h <= 0)
	return;

    WaitQueue(12);
    old_DP_PIX_WIDTH = regr(DP_PIX_WIDTH);
    old_DST_OFF_PITCH = regr(DST_OFF_PITCH);

    regw(SC_LEFT_RIGHT, (((x+w-1) << 16) | (x & 0x0000ffff)));
    regw(SC_TOP_BOTTOM, (((y+h-1) << 16) | (y & 0x0000ffff)));

    regw(DP_WRITE_MASK, planemask);
    regw(DP_FRGD_CLR, fgPixel);
    regw(DP_BKGD_CLR, bgPixel);

    if( opaque )
        regw(DP_MIX, (alu << 16) | alu);
    else
        regw(DP_MIX, (alu << 16) | MIX_DST);

    switch(mach64InfoRec.bitsPerPixel)
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

    w = (w + (px % 32) + 31) >> 5; /* round up to int boundry and take into */
				   /* account the pixels skipped over */

    if (px)
        regw(DST_Y_X, (((x - (px % 32)) << 16) | (y & 0x0000ffff)));
    else
	regw(DST_Y_X, ((x << 16) | (y & 0x0000ffff)));
    regw(DST_HEIGHT_WIDTH, (((w * 32) << 16) | h));

    wordsPerLine = pwidth >> 2;
    pline = (int*)psrc + (wordsPerLine * py) + (px >> 5);

    while (h--) 
    {
        count = w;
	pword = pline;

        while (count >= 16)
        {
#if (BITMAP_BIT_ORDER == MSBFirst)
	    for(i = 15; i >= 0; i--) {
	      reversed_bitmap[i] = bit_reverse32(*pword);
	      ++pword;
	    }
            WaitQueue(16);
            regw(HOST_DATAF, reversed_bitmap[0x0f]);
            regw(HOST_DATAE, reversed_bitmap[0x0e]);
            regw(HOST_DATAD, reversed_bitmap[0x0d]);
            regw(HOST_DATAC, reversed_bitmap[0x0c]);
            regw(HOST_DATAB, reversed_bitmap[0x0b]);
            regw(HOST_DATAA, reversed_bitmap[0x0a]);
            regw(HOST_DATA9, reversed_bitmap[9]);
            regw(HOST_DATA8, reversed_bitmap[8]);
            regw(HOST_DATA7, reversed_bitmap[7]);
            regw(HOST_DATA6, reversed_bitmap[6]);
            regw(HOST_DATA5, reversed_bitmap[5]);
            regw(HOST_DATA4, reversed_bitmap[4]);
            regw(HOST_DATA3, reversed_bitmap[3]);
            regw(HOST_DATA2, reversed_bitmap[2]);
            regw(HOST_DATA1, reversed_bitmap[1]);
            regw(HOST_DATA0, reversed_bitmap[0]);
#else
            WaitQueue(16);
            regw(HOST_DATAF, *pword++);
            regw(HOST_DATAE, *pword++);
            regw(HOST_DATAD, *pword++);
            regw(HOST_DATAC, *pword++);
            regw(HOST_DATAB, *pword++);
            regw(HOST_DATAA, *pword++);
            regw(HOST_DATA9, *pword++);
            regw(HOST_DATA8, *pword++);
            regw(HOST_DATA7, *pword++);
            regw(HOST_DATA6, *pword++);
            regw(HOST_DATA5, *pword++);
            regw(HOST_DATA4, *pword++);
            regw(HOST_DATA3, *pword++);
            regw(HOST_DATA2, *pword++);
            regw(HOST_DATA1, *pword++);
            regw(HOST_DATA0, *pword++);
#endif
	    count -= 16;
        }

#if (BITMAP_BIT_ORDER == MSBFirst)
	for(i = count - 1; i >= 0; i--) {
	  reversed_bitmap[i] = bit_reverse32(*pword);
	  ++pword;
	}
        WaitQueue(count);
        switch(count)
        {
            case 15: regw(HOST_DATAE, reversed_bitmap[0x0e]);
            case 14: regw(HOST_DATAD, reversed_bitmap[0x0d]);
            case 13: regw(HOST_DATAC, reversed_bitmap[0x0c]);
            case 12: regw(HOST_DATAB, reversed_bitmap[0x0b]);
            case 11: regw(HOST_DATAA, reversed_bitmap[0x0a]);
            case 10: regw(HOST_DATA9, reversed_bitmap[9]);
            case  9: regw(HOST_DATA8, reversed_bitmap[8]);
            case  8: regw(HOST_DATA7, reversed_bitmap[7]);
            case  7: regw(HOST_DATA6, reversed_bitmap[6]);
            case  6: regw(HOST_DATA5, reversed_bitmap[5]);
            case  5: regw(HOST_DATA4, reversed_bitmap[4]);
            case  4: regw(HOST_DATA3, reversed_bitmap[3]);
            case  3: regw(HOST_DATA2, reversed_bitmap[2]);
            case  2: regw(HOST_DATA1, reversed_bitmap[1]);
            case  1: regw(HOST_DATA0, reversed_bitmap[0]);
                break;
	}
#else
        WaitQueue(16);
        switch(count)
        {
            case 15: regw(HOST_DATAE, *pword++);
            case 14: regw(HOST_DATAD, *pword++);
            case 13: regw(HOST_DATAC, *pword++);
            case 12: regw(HOST_DATAB, *pword++);
            case 11: regw(HOST_DATAA, *pword++);
            case 10: regw(HOST_DATA9, *pword++);
            case  9: regw(HOST_DATA8, *pword++);
            case  8: regw(HOST_DATA7, *pword++);
            case  7: regw(HOST_DATA6, *pword++);
            case  6: regw(HOST_DATA5, *pword++);
            case  5: regw(HOST_DATA4, *pword++);
            case  4: regw(HOST_DATA3, *pword++);
            case  3: regw(HOST_DATA2, *pword++);
            case  2: regw(HOST_DATA1, *pword++);
            case  1: regw(HOST_DATA0, *pword);
            default:
                break;
        }
#endif
	pline += wordsPerLine;
    }

    WaitQueue(4);

    regw(DST_OFF_PITCH, old_DST_OFF_PITCH);
    regw(DP_PIX_WIDTH, old_DP_PIX_WIDTH);
    regw(SC_LEFT_RIGHT, ((mach64MaxX << 16) | 0 ));
    regw(SC_TOP_BOTTOM, ((mach64MaxY << 16) | 0 ));

    WaitIdleEmpty();
}
