/* $XConsortium: s3im.c,v 1.6 95/01/06 20:57:19 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3im.c,v 3.15 1995/07/12 15:36:46 dawes Exp $ */
/*
 * Copyright 1992 by Kevin E. Martin, Chapel Hill, North Carolina.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Kevin E. Martin not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Kevin E. Martin makes no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * Modified by Amancio Hasty and extensively by Jon Tombs & Phil Richards.
 * 
 * Id: s3im.c,v 2.7 1993/08/10 15:20:03 jon Exp jon
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */


#include	"amiga.h"

#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"mifillarc.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"amigaCV.h"
#include	"migc.h"

#include        "xf86bcache.h"
#include        "xf86fcache.h"
#include        "xf86text.h"  


#define	reorder(a,b)	b = \
	(a & 0x80) >> 7 | \
	(a & 0x40) >> 5 | \
	(a & 0x20) >> 3 | \
	(a & 0x10) >> 1 | \
	(a & 0x08) << 1 | \
	(a & 0x04) << 3 | \
	(a & 0x02) << 5 | \
	(a & 0x01) << 7;


/* fast ImageWrite(), ImageRead(), and ImageFill() routines */
/* there are two cases; (i) when the bank switch can occur in the */
/* middle of raster line, and (ii) when it is guaranteed not possible. */
/* In theory, s3InfoRec.virtualX should contain the number of bytes */
/* on the raster line; however, this is not necessarily true, and for */
/* many situations, the S3 card will always have 1024. */
/* Phil Richards <pgr@prg.ox.ac.uk> */
/* 26th November 1992 */
/* Bug fixed by Jon Tombs */
/* 30/7/94 began 16,32 bit support */

#undef CV_USE_BCOPY

extern void mybcopyas (void * src, void *dst, int n);

#ifdef CV_USE_BCOPY
#define mybcopy(src, dst, n) memcpy (dst,src, n)
#else

/* 
 * Faster ? 
 * 
 * is  written in assembler.
 */

 
#define mybcopy(src, dst, n) asm volatile ("movel %0,%%a0; movel %1,%%a1;\
movel %2, %%d0; jsr _mybcopyas" : : "g" (src), "g" (dst), "g" (n) : \
"a0", "a1", "d0", "d1")
 
#endif




/* For masked reads */
static __inline
void mylcopymasked (volatile unsigned long * src, volatile unsigned long * dst, int n, unsigned long mask)
{
while (n-- > 0)
      *dst++ = *src++ & mask;

}

static __inline
void mywcopymasked (volatile unsigned short * src, volatile unsigned short * dst, int n, unsigned long mask)
{
while (n-- > 0)
      *dst++ = *src++ & mask;

}
static __inline
void mybcopymasked (volatile char * src, volatile char * dst, int n, unsigned long mask)
{
while (n-- > 0)
      *dst++ = *src++ & mask;
}



#define ALIGNMENT 8
#define MAX_PIXMAP_WIDTH 64
#define MIN_PIXMAP_WIDTH 8            
#define MIN_FONTCACHE_HEIGHT 13
#define MIN_FONTCACHE_WIDTH (32 * 6)  

 CachePool FontPool;

unsigned short s3SwapBits[256];

void amigaCVImageInit (fbFd *inf)
{
  int i;
   static int first = 1;

   int h, h2, pmwidth = 0, pmx, pmy;
   int BitPlane;
   int fcwidth, fcheight;
   /* Init swap bits */
   for (i = 0; i < 256; i++) {
    /* reorder (i, s3SwapBits[i]);*/
    s3SwapBits[i] = i;
   }     






   /* init Pixmap cache (below the displayable area of the gfx mem) */



   h = amigaVirtualHeight + 1;

   h2 = inf->info.gd_fbsize/(inf->info.gd_fbwidth * (inf->info.gd_planes/8));

   if (h2-h > MAX_PIXMAP_WIDTH)
     pmwidth = MAX_PIXMAP_WIDTH;
   else 
     pmwidth = h2 - h;

   pmx = 0;
   pmy = h;


   if (pmwidth >= MIN_PIXMAP_WIDTH)
     amigaCVInitFrect(pmx, pmy, pmwidth);
#ifdef USE_FONTCACHE
    fcheight = h2 - h - pmwidth -10 ;
    fcwidth = amigaVirtualWidth - 1;

   if (fcheight >= MIN_FONTCACHE_HEIGHT) {
	if (first) {
	  FontPool = xf86CreateCachePool(ALIGNMENT); 
          for( BitPlane = inf->info.gd_planes-1; BitPlane >= 0; BitPlane-- ) {
            xf86AddToCachePool(FontPool, 0, h+pmwidth, 
		fcwidth ,fcheight , 1<<BitPlane );
          }  
          xf86InitFontCache(FontPool, fcwidth , 
		fcheight , s3FontStipple);
          xf86InitText(s3GlyphWrite, s3NoCPolyText, s3NoCImageText );
        } else {
          xf86ReleaseFontCache();
        }
   } else if (first) {

      /*
       * Crash and burn if the cached glyph write function gets called.
       */
      xf86InitText( NULL, s3NoCPolyText, s3NoCImageText );
      ErrorF( "No font cache available\n" );
   }      

#endif /* USE_FONTCACHE */
   first = 0;
   xf86InitCache(s3CacheMoveBlock);   

}




 void
amigaCVImageWrite (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   px,
     int   py,
     short alu,
     unsigned long planemask, fbFd *inf)
{
   int   j, offset;
   int depth = inf->info.gd_planes;
   volatile caddr_t vgaBase = (inf->regs);
   int s3Bpp  /*= depth>>3 */; /* Byte per pixel */
   volatile char *videobuffer = inf->fb;
   int s3BppDisplayWidth /*= inf->info.gd_fbwidth * s3Bpp*/;
   unsigned long s3BppPMask  /* = (1UL << depth) -1 */;
   int s3Shift; /* x * s3Bpp == x << s3Shift ! Faster ! */

   if (w == 0 || h == 0)
      return;

   switch (depth) {
   case 24: case 32:
     s3Bpp = 4;
     s3BppPMask =0xffffffff;
     s3Shift = 2; 
     break;
   case 16:
     s3Bpp = 2;
     s3BppPMask =0xffff;
     s3Shift = 1;
     break;
   case 8:
   default:
     s3Bpp = 1;
     s3BppPMask = 0xff;
     s3Shift = 0;
   }

   s3BppDisplayWidth = inf->info.gd_fbwidth << s3Shift;  



   if (alu == MIX_DST)
      return;
   if (w == 0 || h == 0)
      return;

   if ((alu != MIX_SRC) || ((planemask & s3BppPMask) != s3BppPMask)) {
      amigaCVImageWriteNoMem(x, y, w, h, psrc, pwidth, px, py, alu, planemask, inf);
      return;
   }

   BLOCK_CURSOR;
#if 0
   WaitQueue16_32(2,3);
   outw (FRGD_MIX, FSS_PCDATA | alu);
   outw32(WRT_MASK, planemask);
#endif

   psrc += pwidth * py + (px << s3Shift);
   offset = (y * s3BppDisplayWidth) + (x << s3Shift);

   w <<= s3Shift;
  WaitIdleEmpty();
   
   for (j = 0; j < h; j++, psrc += pwidth, offset += s3BppDisplayWidth) {
      mybcopy (psrc, &videobuffer[offset], w);
   }

#if 0
   WaitQueue(1);
   outw (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
#endif
   UNBLOCK_CURSOR;
}

 void
amigaCVImageRead (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   px,
     int   py,
     unsigned long planemask,
     fbFd *inf)
{
   int   j, offset;
   int depth = inf->info.gd_planes;
   int s3Bpp /* = depth>>3 */; /* Byte per pixel */
   volatile char *videobuffer = inf->fb;
   volatile caddr_t vgaBase = (inf->regs);
   int s3BppDisplayWidth /*= inf->info.gd_fbwidth * s3Bpp*/;
   unsigned long s3BppPMask  /* = (1UL << depth) -1 */;
   int s3Shift; /* x * s3Bpp == x << s3Shift ! Faster ! */

   if (w == 0 || h == 0)
      return;

   switch (depth) {
   case 24: case 32:
     s3Bpp = 4;
     s3BppPMask =0xffffffff;
     s3Shift = 2; 
     break;
   case 16:
     s3Bpp = 2;
     s3BppPMask =0xffff;
     s3Shift = 1;
     break;
   case 8:
   default:
     s3Bpp = 1;
     s3BppPMask = 0xff;
     s3Shift = 0;
   }

   s3BppDisplayWidth = inf->info.gd_fbwidth << s3Shift;  



/* We can't use ReadNoMem, it doesn`t work with trio64 :-(*/
#if 0
   if ((planemask & s3BppPMask) != s3BppPMask) {
      amigaCVImageReadNoMem(x, y, w, h, psrc, pwidth, px, py, planemask);
      return;
   }
#endif

      

#if 0
   outw (FRGD_MIX, FSS_PCDATA | MIX_SRC);
#endif
 
   psrc += pwidth * py + (px << s3Shift);
   offset = (y * s3BppDisplayWidth) + (x << s3Shift);

   w <<= s3Shift;

   BLOCK_CURSOR;
   WaitIdle ();

   if ((planemask & s3BppPMask) != s3BppPMask) {
     for (j = 0; j < h; j++, psrc += pwidth, offset += s3BppDisplayWidth) {
       switch (depth) {
         case 8:
	 mybcopymasked ( &videobuffer[offset], psrc, w, planemask);
	 break;
         case 16:
	 mywcopymasked ((volatile unsigned short *) &videobuffer[offset],
			(volatile unsigned short *) psrc, w>>1, planemask);
	 break;       
         case 32:
	 mylcopymasked ((volatile unsigned long *)&videobuffer[offset], 
			(volatile unsigned long *) psrc, w>>2, planemask);
	 break;
       }
     }
   } else
     for (j = 0; j < h; j++, psrc += pwidth, offset += s3BppDisplayWidth)
         mybcopy ( &videobuffer[offset],psrc, w);
   



#if 0
   WaitQueue(1);
   outw (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
#endif
   UNBLOCK_CURSOR;
}



void
amigaCVImageFill (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   pw, 
     int   ph, 
     int   pox,
     int   poy,
     short alu,
     unsigned long planemask,
     fbFd *inf)
{
   int   j;
   char *pline;
   int   ypix, xpix, offset0;
   int   cxpix;

   int depth = inf->info.gd_planes;
   int s3Bpp = depth>>3; /* Byte per pixel */
   volatile caddr_t vgaBase = (inf->regs);
   volatile char *videobuffer = inf->fb;
   int s3BppDisplayWidth = inf->info.gd_fbwidth * s3Bpp;
   unsigned long s3BppPMask = (1UL << depth) -1;

   if (alu == MIX_DST)
      return;

   if ((alu != MIX_SRC) || ((planemask & s3BppPMask) != s3BppPMask)) {
     amigaCVImageFillNoMem(x, y, w, h, psrc, pwidth,
                    pw, ph, pox, poy, alu, planemask, inf);
     return;
   }

   if (w == 0 || h == 0)
      return;
      
 
#if 0
   WaitQueue16_32(2,3);
   outw (FRGD_MIX, FSS_PCDATA | alu);
   outw32(WRT_MASK, planemask);
#endif

   w  *= s3Bpp;
   pw *= s3Bpp;

   modulus ((x - pox) * s3Bpp, pw, xpix);
   cxpix = pw - xpix ;

   modulus (y - poy, ph, ypix);
   pline = psrc + pwidth * ypix;

   offset0 = (y * s3BppDisplayWidth) + x * s3Bpp;


   BLOCK_CURSOR;
   WaitIdle();   

   for (j = 0; j < h; j++, offset0 += s3BppDisplayWidth) {
      if (w <= cxpix) {
	 mybcopy (pline + xpix, &videobuffer[offset0], w);
      } else {
	 int   width, offset;

	 mybcopy (pline + xpix, &videobuffer[offset0], cxpix);

	 offset = offset0 + cxpix;
	 for (width = w - cxpix; width >= pw; width -= pw, offset += pw)
	    mybcopy (pline, &videobuffer[offset], pw);

       /* at this point: 0 <= width < pw */
	 if (width > 0)
	    mybcopy (pline, &videobuffer[offset], width);
      }

      if ((++ypix) == ph) {
	 ypix = 0;
	 pline = psrc;
      } else
	 pline += pwidth;
   }

#if 0
   WaitQueue(1);
   outw (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
#endif
   UNBLOCK_CURSOR;
}





void
amigaCVImageWriteNoMem (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   px,
     int   py,
     short alu,
     unsigned long planemask,
     fbFd *inf)
{
   int   i, j;
   int depth = inf->info.gd_planes;
   int s3Bpp = depth>>3; /* Byte per pixel */
   volatile caddr_t vgaBase = (inf->regs);

   if (alu == MIX_DST)
      return;

   if (w == 0 || h == 0)
      return;
      
   BLOCK_CURSOR;
   WaitQueue16_32(2,3); PCI_HACK();
   S3_OUTW (FRGD_MIX, FSS_PCDATA | alu);
   S3_OUTW32 (WRT_MASK, planemask);
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | 0);

   WaitQueue(4);PCI_HACK();
   S3_OUTW (CUR_X, (short) x);
   S3_OUTW (CUR_Y, (short) y);
   S3_OUTW (MAJ_AXIS_PCNT, (short) w - 1);
   S3_OUTW (MULTIFUNC_CNTL, MIN_AXIS_PCNT | (h - 1));
   WaitIdle();
   S3_OUTW (CMD, CMD_RECT /*| BYTSEQ*/ | _16BIT | INC_Y | INC_X | DRAW | PCDATA
	  | WRTDATA);

   w *= s3Bpp;
   psrc += pwidth * py;

   for (j = 0; j < h; j++) {
      /* This assumes we can cast odd addresses to short! */
      short *psrcs = (short *)&psrc[px*s3Bpp];
      for (i = 0; i < w; ) {
	 if (depth == 32) {
	    S3_OUTW (PIX_TRANS, *psrcs++);
	    S3_OUTW (PIX_TRANS, *psrcs++);
	    i += 4;
	 }
	 else {
	    S3_OUTW (PIX_TRANS, *psrcs++);
	    i += 2;
	 }
      }
      psrc += pwidth;
   }
   WaitQueue16_32(2,3); 
   WaitIdle();
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW32(WRT_MASK, ~0);
   UNBLOCK_CURSOR;
}



void
amigaCVImageReadNoMem (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   px,
     int   py,
     unsigned long planemask,
     fbFd* inf)
{
   int   i, j;
   int depth = inf->info.gd_planes;
   int s3Bpp = depth>>3; /* Byte per pixel */
   volatile caddr_t vgaBase = (inf->regs);

   if (w == 0 || h == 0)
      return;

__dolog ("ImageRead1: depth %d planemask %ld", depth, planemask);      
   BLOCK_CURSOR;
   WaitIdleEmpty ();
   WaitQueue(7);PCI_HACK();
   S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL);
   S3_OUTW (FRGD_MIX, FSS_PCDATA | MIX_SRC);
   S3_OUTW (CUR_X, (short) x);
   S3_OUTW (CUR_Y, (short) y);
   S3_OUTW (MAJ_AXIS_PCNT, (short) w - 1);
   S3_OUTW (MULTIFUNC_CNTL, MIN_AXIS_PCNT | (h - 1));
   S3_OUTW (CMD, CMD_RECT /* | BYTSEQ*/ | _16BIT | INC_Y | INC_X | DRAW |
	  PCDATA | WRTDATA);


  WaitQueue16_32(1,2);

   S3_OUTW32(RD_MASK, planemask);

  WaitQueue(8);

 /* wait for data to be ready */

/*   while ((vgar16 (vgaBase, GP_STAT) & 0x100) == 0) ;*/

   w *= s3Bpp;
   psrc += pwidth * py;

   for (j = 0; j < h; j++) {
      short *psrcs = (short *)&psrc[px*s3Bpp]; 
      for (i = 0; i < w; ) {
	 if (depth == 32) {
	    int tmp;
	    /* XXX don't know which one first :-)*/
	    tmp = vgar16(vgaBase,PIX_TRANS);
	    *psrcs++ = tmp;

	    tmp = vgar16(vgaBase,PIX_TRANS);
	    *psrcs++ = tmp<<16;
	    /*psrcs += 2;*/
	    i += 4;
	 } else {
	    *psrcs++ = vgar16(vgaBase, PIX_TRANS);
	    __dolog (" %x ", *(psrcs -1) );
	    i += 2;
	 }
      }
      psrc += pwidth;
   }
__dolog("ImageRead3");
   WaitQueue16_32(2,3); 
   WaitIdle();
   S3_OUTW32(RD_MASK, ~0);
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;
}


 void
amigaCVImageFillNoMem (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   pw,
     int   ph,
     int   pox,
     int   poy,
     short alu,
     unsigned long planemask,
     fbFd *inf)
{
   int   i, j;
   char *pline;
   int   mod;
   int depth = inf->info.gd_planes;
   int s3Bpp = depth>>3; /* Byte per pixel */
      volatile caddr_t vgaBase =  (inf->regs);

   if (alu == MIX_DST)
      return;

   if (w == 0 || h == 0)
      return;

   BLOCK_CURSOR;  
   WaitQueue16_32(6,7); PCI_HACK();
   S3_OUTW (FRGD_MIX, FSS_PCDATA | alu);
   S3_OUTW32 (WRT_MASK, planemask);
   S3_OUTW (CUR_X, (short) x);
   S3_OUTW (CUR_Y, (short) y);
   S3_OUTW (MAJ_AXIS_PCNT, (short) w - 1);
   S3_OUTW (MULTIFUNC_CNTL, MIN_AXIS_PCNT | (h - 1));
   WaitIdle();
   S3_OUTW (CMD, CMD_RECT |/* BYTSEQ| */ _16BIT | INC_Y | INC_X | DRAW |
	 PCDATA | WRTDATA);

   for (j = 0; j < h; j++) {
      unsigned short wrapped;
      unsigned short *pend;
      unsigned short *plines;

      modulus (y + j - poy, ph, mod);
      pline = psrc + pwidth * mod;
      pend = (unsigned short *)&pline[pw*s3Bpp];
      wrapped = (pline[0] << 8) + (pline[pw-1] << 0); /* only for 8bpp */

      modulus (x - pox, pw, mod);

      plines = (unsigned short *) &pline[mod*s3Bpp];

      for (i = 0; i < w * s3Bpp;)  {

         /* arrg in 8BPP we need to check for wrap round */
         if (plines + 1 > pend) {
	    S3_OUTW (PIX_TRANS, wrapped);
            plines = (unsigned short *)&pline[1]; i += 2;
	 } else {
	    if (depth == 32) {
	       S3_OUTW (PIX_TRANS, *plines++);
	       S3_OUTW (PIX_TRANS, *plines++);
	       i += 4;
	    }
	    else {
	       S3_OUTW (PIX_TRANS, *plines++);
	       i += 2;
	    }
	 }
	 if (plines == pend)
	    plines = (unsigned short *)pline;
	 
      }
   }

   WaitQueue(1); WaitIdle();
   S3_OUTW (FRGD_MIX, FSS_FRGDCOL | MIX_SRC);   
   UNBLOCK_CURSOR;
}



#if 0
static int _internal_s3_mskbits[17] =
{
   0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff,
   0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
#else

static int _internal_s3_mskbits[17] =
{
   0x0000, 0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00, 
   0xff80, 0xffc0, 0xffe0,
   0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff
};
#endif

#define MSKBIT(n) (_internal_s3_mskbits[(n)])

static void
amigaCVRealImageStipple(
    int			x,
    int			y,
    int			w,
    int			h,
    unsigned char	*psrc,
    int			pw,
    int			ph,
    int			pox,
    int			poy,
    int			pwidth,
    Pixel		fgPixel,
    Pixel		bgPixel,
    short		alu,
    Pixel		planemask,
    int			opaque,
    fbFd               *inf)
{
    int			srcx, srch, dstw;
    unsigned char 	*ptmp;
   int depth = inf->info.gd_planes;
   int s3Bpp = depth>>3; /* Byte per pixel */
      volatile caddr_t vgaBase = (inf->regs);


    if (alu == MIX_DST || w == 0 || h == 0)
	return;

    BLOCK_CURSOR;
    WaitQueue16_32(5,8); PCI_HACK();
    S3_OUTW32 (WRT_MASK, planemask);
    S3_OUTW (FRGD_MIX, FSS_FRGDCOL | alu);
    if( opaque ) {
      S3_OUTW (BKGD_MIX, BSS_BKGDCOL | alu);
      S3_OUTW32 (BKGD_COLOR,  bgPixel);
    }
    else
      S3_OUTW (BKGD_MIX, BSS_BKGDCOL | MIX_DST);

    S3_OUTW32 (FRGD_COLOR,  fgPixel);
    WaitQueue(5); PCI_HACK();
    S3_OUTW (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPPC | COLCMPOP_F);
    S3_OUTW (MAJ_AXIS_PCNT, (short) (w - 1));
    S3_OUTW (CUR_X, (short) x);
    S3_OUTW (CUR_Y, (short) y);
    S3_OUTW (MULTIFUNC_CNTL, MIN_AXIS_PCNT | (h-1));   
    WaitIdle();
    S3_OUTW (CMD, CMD_RECT | PCDATA | _16BIT | INC_Y | INC_X |
	     DRAW | PLANAR | WRTDATA | BYTSEQ);

    modulus(x - pox, pw, x);
    modulus(y - poy, ph, y);


    __dolog ("Stipple: x %d, y %d, w %d, h %d\n", x, y, w, h);

    /*
     * When the source bitmap is properly aligned, max 16 pixels wide
     * and nonrepeating, use this faster loop instead.
     */
    if( (x & 7) == 0 && w <= 16 && x+w <= pw && y+h <= ph ) {
	/*unsigned short pix;*/
	unsigned char *pnt;

        __dolog ("short loop!\n");
	pnt = (unsigned char *)(psrc + pwidth * y + (x >> 3));
	while( h-- > 0 ) {
/*	    pix = *((unsigned short *)(pnt));*/
#if 0
	    S3_OUTW(PIX_TRANS, s3SwapBits[ pix & 0xff ] | 
			       s3SwapBits[ ( pix >> 8 ) & 0xff ] << 8);
#else
	    S3_OUTW(PIX_TRANS, s3SwapBits[ pnt[1]  ] << 8| 
			       s3SwapBits[ ( pnt[0]  ) & 0xff ] );
#endif
	    pnt += pwidth;
	}
    }
    else {
	while( h > 0 ) {
	    srch = ( y+h > ph ? ph - y : h );
	    while( srch > 0 ) {
		dstw = w;
		srcx = x;
		ptmp = psrc + pwidth * y;
		while( dstw > 0 ) {
		    int np, x2;
		    unsigned char *pnt;
		    unsigned short pix;
		    unsigned long pixtemp;
		    /*
		     * Assemble 16 bits and feed them to the draw engine.
		     */
		    np = pw - srcx;		/* No. pixels left in bitmap.*/
		    pnt = ptmp + (srcx >> 3);
		    x2 = srcx & 7;		/* Offset within byte. */
		    if( np >= 16 ) {
/*			pix = (unsigned short)(*((unsigned long *)(pnt)) >> x2);*/
			pixtemp = (pnt[0]<<24)+ (pnt[1] << 16)+ (pnt[2] << 8) + (pnt[3]);
			pixtemp <<=x2;
			pix = pixtemp >> 16;
		    }
		    else if( pw >= 16 ) {
/*			pix = (unsigned short)((*((unsigned long *)(pnt)) >> x2)
						 & MSKBIT(np)) | (*ptmp << np);*/
                        pixtemp = (pnt[0]<<24)+ (pnt[1] << 16)+ (pnt[2] << 8) + (pnt[3]);
                        pixtemp <<=x2;
			pix = (pixtemp >> 16) & MSKBIT(np) ;
			pixtemp = (ptmp[0] << 8) + ptmp [1];
			pix |= pixtemp >> np;
		    }
		    else if( pw >= 8 ) {
/*			pix = ((*pnt >> x2) & MSKBIT(np)) | (*ptmp << np)
						      | (*pnt << (np+pw));*/
			pixtemp = (pnt[0]<<8)+ (pnt[1]);
			pixtemp <<=x2;
			pix = pixtemp & MSKBIT(np);
			pixtemp = (ptmp[0] << 8) + ptmp [1];
			pix |=pixtemp >> np;
			pixtemp = (pnt[0]<<8)+ (pnt[1]);
			pix |= pixtemp >> (np+pw);

		    }
		    else {
			/*pix = (*ptmp >> x2) & MSKBIT(np);*/
			pixtemp = (pnt[0]<<8)+ (pnt[1]);
			pix = (pixtemp << x2) & MSKBIT(np);
 
			while( np < 16 && np < dstw ) {
			    pixtemp = (ptmp[0] << 8) + ptmp [1];
			    /*pix |= *ptmp << np;*/
			    pix |= pixtemp >> np;
			    np += pw;
			}
		    }
#if 0

		    S3_OUTW(PIX_TRANS, s3SwapBits[ pix & 0xff ] | 
				       s3SwapBits[ ( pix >> 8 ) & 0xff ] << 8);
#else

		    S3_OUTW(PIX_TRANS, s3SwapBits[ pix & 0xff ] << 8 | 
				       s3SwapBits[ ( pix >> 8 ) & 0xff ]);
#endif
		    srcx += 16;
		    if( srcx >= pw )
			srcx -= pw;
		    dstw -= 16;
		}
		y++;
		h--;
		srch--;
	    }
	    y = 0;
	}
    }
    WaitQueue(3); WaitIdle();
    S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
    S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
    S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
    UNBLOCK_CURSOR;
}

void
amigaCVImageStipple (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   pw,
     int   ph,
     int   pox,
     int   poy,
     Pixel fgPixel,
     short alu,
     unsigned long planemask,
     fbFd *inf)
{

    amigaCVRealImageStipple(x, y, w, h, psrc, pwidth, pw, ph, pox, poy,
		       fgPixel, 0, alu, planemask, 0, inf);
}

void
amigaCVImageOpStipple (
     int   x,
     int   y,
     int   w,
     int   h,
     char *psrc,
     int   pwidth,
     int   pw,
     int   ph,
     int   pox,
     int   poy,
     Pixel fgPixel,
     Pixel bgPixel, 
     short alu,
     unsigned long planemask,
     fbFd *inf)
{

    amigaCVRealImageStipple(x, y, w, h, psrc, pwidth, pw, ph, pox, poy,
		       fgPixel, bgPixel, alu, planemask, 1, inf);
}
