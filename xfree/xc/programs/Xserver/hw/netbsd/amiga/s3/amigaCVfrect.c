/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3frect.c,v 3.6 1995/12/23 09:38:34 dawes Exp $ */
/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE. 

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)

KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

*/

/*
 * Snarfed Hans Nasten's simple pixmap expansion cache from Mach-8 server
 * -- David Wexelblat <dwex@xfree86.org>, July 12, 1994
 */
/* $XConsortium: s3frect.c /main/5 1995/12/29 10:30:58 kaleb $ */

/*
 * Modified for the Cybervision 64 by Michael Teske
 */

/*
 * Fill rectangles.
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
#include	"mispans.h"
#include	"migc.h"

extern int cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;

extern RegionPtr cfb8CopyArea(), cfb16CopyArea(), cfb32CopyArea();
extern RegionPtr cfb8CopyPlane(), cfb16CopyPlane(), cfb32CopyPlane();

extern RegionPtr cfb8BitBlt(), cfb16BitBlt(), cfb32BitBlt();


/* Externs from amigaCV.c */

extern short s3alu[];



int	amigaCVScreenPrivateIndex;
int	amigaCVGCPrivateIndex;
int	amigaCVWindowPrivateIndex;
int	amigaCVGeneration;

#define NUM_STACK_RECTS       1024  


typedef struct _CacheInfo {
    int x;
    int y;
    int w;
    int h;
    int nx;
    int ny;
    int pix_w;
    int pix_h;
} CacheInfo, *CacheInfoPtr;

static CacheInfo cInfo;
static int pixmap_x;
static int pixmap_y;
static int pixmap_size = 0;

void amigaCVInitFrect(x, y, size)
     int x;
     int y;
     int size;
{
   pixmap_x = x;
   pixmap_y = y;
   pixmap_size = size;
}


static void
DoCacheExpandPixmap(CacheInfoPtr pci, fbFd *inf)

{
   int   cur_w = pci->pix_w;
   int   cur_h = pci->pix_h;
   volatile caddr_t vgaBase = (inf->regs);

   BLOCK_CURSOR;
   WaitQueue16_32(7,8); PCI_HACK();
#if 0
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (s3DisplayWidth-1));
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | s3ScissB);
#endif
   S3_OUTW(FRGD_MIX, FSS_BITBLT | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   S3_OUTW32(WRT_MASK, ~0);

 /* Expand in the x direction */

   while (cur_w * 2 <= pci->w) {
      WaitQueue(7);PCI_HACK();
      S3_OUTW(CUR_X, (short)pci->x);
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)(pci->x + cur_w));
      S3_OUTW(DESTY_AXSTP, (short)pci->y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)(cur_w - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(cur_h - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

      cur_w *= 2;
   }

   if (cur_w != pci->w) {
      WaitQueue(7);PCI_HACK();
      S3_OUTW(CUR_X, (short)pci->x);
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)(pci->x + cur_w));
      S3_OUTW(DESTY_AXSTP, (short)pci->y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - cur_w - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(cur_h - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

      cur_w = pci->w;
   }
 /* Expand in the y direction */
   while (cur_h * 2 <= pci->h) {
      WaitQueue(7);PCI_HACK();
      S3_OUTW(CUR_X, (short)pci->x);
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)pci->x);
      S3_OUTW(DESTY_AXSTP, (short)(pci->y + cur_h));
      S3_OUTW(MAJ_AXIS_PCNT, (short)(cur_w - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(cur_h - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

      cur_h *= 2;
   }

   if (cur_h != pci->h) {
      WaitQueue(7);PCI_HACK();
      S3_OUTW(CUR_X, (short)pci->x);
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)pci->x);
      S3_OUTW(DESTY_AXSTP, (short)(pci->y + cur_h));
      S3_OUTW(MAJ_AXIS_PCNT, (short)(cur_w - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - cur_h - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);
   }
   WaitQueue(2);
   WaitIdle();
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;
}

static CacheInfoPtr
DoCacheTile(PixmapPtr pix, fbFd *inf)
{
   CacheInfoPtr pci = &cInfo;
   volatile caddr_t vgaBase =  (inf->regs);


   if( (pixmap_size) && (pix->drawable.width <= pixmap_size) &&
       (pix->drawable.height <= pixmap_size) ) {
      pci->pix_w = pix->drawable.width;
      pci->pix_h = pix->drawable.height;
      pci->nx = pixmap_size / pix->drawable.width;
      pci->ny = pixmap_size / pix->drawable.height;
      pci->x = pixmap_x;
      pci->y = pixmap_y;
      pci->w = pci->nx * pci->pix_w;
      pci->h = pci->ny * pci->pix_h;
      amigaCVImageWrite (pci->x, pci->y, pci->pix_w, pci->pix_h,
		          pix->devPrivate.ptr, pix->devKind, 0, 0,
		          MIX_SRC, ~0, inf);

      DoCacheExpandPixmap(pci, inf);
      WaitIdleEmpty(); /* Make sure that all commands have finished */
      return(pci);
   }
   else
      return(NULL);
}


static CacheInfoPtr
DoCacheOpStipple(PixmapPtr pix, fbFd *inf)
{
   CacheInfoPtr pci = &cInfo;
   volatile caddr_t vgaBase =  (inf->regs);

   if( (pixmap_size) && (pix->drawable.width <= pixmap_size) &&
       (pix->drawable.height <= pixmap_size) ) {
      pci->pix_w = pix->drawable.width;
      pci->pix_h = pix->drawable.height;
      pci->nx = pixmap_size / pix->drawable.width;
      pci->ny = pixmap_size / pix->drawable.height;
      pci->x = pixmap_x;
      pci->y = pixmap_y;
      pci->w = pci->nx * pci->pix_w;
      pci->h = pci->ny * pci->pix_h;

      amigaCVImageOpStipple(pci->x, pci->y, pci->pix_w, pci->pix_h,
		       pix->devPrivate.ptr, pix->devKind,
		       pci->pix_w, pci->pix_h, pci->x, pci->y,
		       255, 0, MIX_SRC, ~0, inf);

      DoCacheExpandPixmap(pci, inf);
      WaitIdleEmpty(); /* Make sure that all commands have finished */   
      return(pci);
   }
   else
      return(NULL);
}

static void
DoCacheImageFill(
     CacheInfoPtr pci,
     int   x,
     int   y,
     int   w,
     int   h,
     int   pox,
     int   poy,
     short fgalu,
     short bgalu,
     short fgmix,
     short bgmix,
     Pixel planemask,
     fbFd * inf)

{
   int   xwmid, ywmid, orig_xwmid;
   int   startx, starty, endx, endy;
   int   orig_x = x;
   volatile caddr_t vgaBase =  (inf->regs);

   if (w == 0 || h == 0)
      return;

   modulus(x - pox, pci->w, startx);
   modulus(y - poy, pci->h, starty);
   modulus(x - pox + w - 1, pci->w, endx);
   modulus(y - poy + h - 1, pci->h, endy);

   orig_xwmid = xwmid = w - (pci->w - startx + endx + 1);
   ywmid = h - (pci->h - starty + endy + 1);

   BLOCK_CURSOR;
   WaitQueue16_32(7,8); PCI_HACK();
#if 0
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (s3DisplayWidth-1));
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | s3ScissB);
#endif
   S3_OUTW(FRGD_MIX, fgmix | fgalu);
   S3_OUTW(BKGD_MIX, bgmix | bgalu);
   S3_OUTW32(WRT_MASK,  planemask);

   if (starty + h - 1 < pci->h) {
      if (startx + w - 1 < pci->w) {
	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pci->x + startx));
	 S3_OUTW(CUR_Y, (short)(pci->y + starty));
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(w - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(h - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);
      } else {
	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pci->x + startx));
	 S3_OUTW(CUR_Y, (short)(pci->y + starty));
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - startx - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(h - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

	 x += pci->w - startx;

	 while (xwmid > 0) {
	    WaitQueue(7); PCI_HACK();
	    S3_OUTW(CUR_X, (short)pci->x);
	    S3_OUTW(CUR_Y, (short)(pci->y + starty));
	    S3_OUTW(DESTX_DIASTP, (short)x);
	    S3_OUTW(DESTY_AXSTP, (short)y);
	    S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - 1));
	    S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(h - 1));
	    S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR |
		  WRTDATA);
	    x += pci->w;
	    xwmid -= pci->w;
	 }

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)pci->x);
	 S3_OUTW(CUR_Y, (short)(pci->y + starty));
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)endx);
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(h - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);
      }
   } else if (startx + w - 1 < pci->w) {
      WaitQueue(7); PCI_HACK();
      S3_OUTW(CUR_X, (short)(pci->x + startx));
      S3_OUTW(CUR_Y, (short)(pci->y + starty));
      S3_OUTW(DESTX_DIASTP, (short)x);
      S3_OUTW(DESTY_AXSTP, (short)y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)(w - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - starty - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

      y += pci->h - starty;

      while (ywmid > 0) {
	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pci->x + startx));
	 S3_OUTW(CUR_Y, (short)pci->y);
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(w - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

	 y += pci->h;
	 ywmid -= pci->h;
      }

      WaitQueue(7);PCI_HACK(); 
      S3_OUTW(CUR_X, (short)(pci->x + startx));
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)x);
      S3_OUTW(DESTY_AXSTP, (short)y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)(w - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)endy);
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);
   } else {
      WaitQueue(7); PCI_HACK();
      S3_OUTW(CUR_X, (short)(pci->x + startx));
      S3_OUTW(CUR_Y, (short)(pci->y + starty));
      S3_OUTW(DESTX_DIASTP, (short)x);
      S3_OUTW(DESTY_AXSTP, (short)y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - startx - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - starty - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

      x += pci->w - startx;

      while (xwmid > 0) {
	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)pci->x);
	 S3_OUTW(CUR_Y, (short)(pci->y + starty));
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT |
	       (short)(pci->h - starty - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

	 x += pci->w;
	 xwmid -= pci->w;
      }

      WaitQueue(7); PCI_HACK();
      S3_OUTW(CUR_X, (short)pci->x);
      S3_OUTW(CUR_Y, (short)(pci->y + starty));
      S3_OUTW(DESTX_DIASTP, (short)x);
      S3_OUTW(DESTY_AXSTP, (short)y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)endx);
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - starty - 1));
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

      y += pci->h - starty;

      while (ywmid > 0) {
	 x = orig_x;
	 xwmid = orig_xwmid;

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pci->x + startx));
	 S3_OUTW(CUR_Y, (short)pci->y);
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - startx - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

	 x += pci->w - startx;

	 while (xwmid > 0) {
	    WaitQueue(7); PCI_HACK();
	    S3_OUTW(CUR_X, (short)pci->x);
	    S3_OUTW(CUR_Y, (short)pci->y);
	    S3_OUTW(DESTX_DIASTP, (short)x);
	    S3_OUTW(DESTY_AXSTP, (short)y);
	    S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - 1));
	    S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - 1));
	    S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR |
		  WRTDATA);

	    x += pci->w;
	    xwmid -= pci->w;
	 }

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)pci->x);
	 S3_OUTW(CUR_Y, (short)pci->y);
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)endx);
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pci->h - 1));
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

	 y += pci->h;
	 ywmid -= pci->h;
      }

      x = orig_x;
      xwmid = orig_xwmid;

      WaitQueue(7); PCI_HACK();
      S3_OUTW(CUR_X, (short)(pci->x + startx));
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)x);
      S3_OUTW(DESTY_AXSTP, (short)y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - startx - 1));
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)endy);
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);      

      x += pci->w - startx;

      while (xwmid > 0) {
	 WaitQueue(7);  PCI_HACK();
	 S3_OUTW(CUR_X, (short)pci->x);
	 S3_OUTW(CUR_Y, (short)pci->y);
	 S3_OUTW(DESTX_DIASTP, (short)x);
	 S3_OUTW(DESTY_AXSTP, (short)y);
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pci->w - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)endy);
	 S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

	 x += pci->w;
	 xwmid -= pci->w;
      }

      WaitQueue(7); PCI_HACK();
      S3_OUTW(CUR_X, (short)pci->x);
      S3_OUTW(CUR_Y, (short)pci->y);
      S3_OUTW(DESTX_DIASTP, (short)x);
      S3_OUTW(DESTY_AXSTP, (short)y);
      S3_OUTW(MAJ_AXIS_PCNT, (short)endx);
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)endy);
      S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);
   }

   WaitQueue(2); 
   WaitIdle();
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;
}

static void
amigaCVCImageFill(
     CacheInfoPtr pci,
     int   x,
     int   y,
     int   w,
     int   h,
     int   pox,
     int   poy,
     short alu,
     Pixel planemask,
     fbFd  *inf)
{

   DoCacheImageFill(pci, x, y, w, h, pox, poy, alu, 
		    MIX_SRC, FSS_BITBLT, BSS_BITBLT, planemask, inf);

}

static void
amigaCVCImageStipple(
     CacheInfoPtr pci,
     int   x,
     int   y,
     int   w,
     int   h,
     int   pox,
     int   poy,
     Pixel  fg,
     short alu,
     Pixel planemask,
     fbFd * inf)
{
   volatile caddr_t vgaBase = (inf->regs);

   BLOCK_CURSOR;
   WaitQueue16_32(3,5);PCI_HACK();
   S3_OUTW32(FRGD_COLOR, fg);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPBLT | COLCMPOP_F);
   S3_OUTW32(RD_MASK, 0x01);

   DoCacheImageFill(pci, x, y, w, h, pox, poy, alu, 
		    MIX_DST, FSS_FRGDCOL, BSS_BKGDCOL, planemask, inf);

   WaitQueue16_32(2,3);PCI_HACK();
   S3_OUTW32(RD_MASK, ~0);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   UNBLOCK_CURSOR;
}

static void
amigaCVCImageOpStipple(
     CacheInfoPtr pci,
     int   x,
     int   y,
     int   w,
     int   h,
     int   pox,
     int   poy,
     Pixel  fg,
     Pixel   bg,
     short alu,
     Pixel planemask,
     fbFd * inf)
{
   volatile caddr_t vgaBase = (inf->regs);

   BLOCK_CURSOR;
   WaitQueue16_32(4,7);PCI_HACK();
   S3_OUTW32(FRGD_COLOR, fg);
   S3_OUTW32(BKGD_COLOR, bg);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPBLT | COLCMPOP_F);
   S3_OUTW32(RD_MASK, 0x01);

   DoCacheImageFill(pci, x, y, w, h, pox, poy, alu, alu, 
		    FSS_FRGDCOL, BSS_BKGDCOL, planemask, inf);

   WaitQueue16_32(2,3);PCI_HACK();
   S3_OUTW32(RD_MASK, ~0);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   UNBLOCK_CURSOR;
}



void
amigaCVPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
     DrawablePtr pDrawable;
     register GCPtr pGC;
     int   nrectFill;		/* number of rectangles to fill */
     xRectangle *prectInit;	/* Pointer to first rectangle to fill */
{
   xRectangle *prect;
   RegionPtr prgnClip;
   register BoxPtr pbox;
   register BoxPtr pboxClipped;
   BoxPtr pboxClippedBase;
   BoxPtr pextent;
   BoxRec stackRects[NUM_STACK_RECTS];
   cfbPrivGC *priv;
   int   numRects;
   int   n;
   int   xorg, yorg;
   int   width, height;
   PixmapPtr pPix;
   int   pixWidth;
   int   xrot, yrot;
   CacheInfoPtr pci; 

   fbFd *inf = amigaInfo(pDrawable->pScreen);
   volatile caddr_t vgaBase = (inf->regs);



   if (pDrawable->type != DRAWABLE_WINDOW) {
     return (miPolyFillRect(pDrawable, pGC, nrectFill, prectInit));
   }

   priv = (cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr;
   prgnClip = cfbGetCompositeClip(pGC);

   prect = prectInit;
   xorg = pDrawable->x;
   yorg = pDrawable->y;
   if (xorg || yorg) {
      prect = prectInit;
      n = nrectFill;
      while (n--) {
	 prect->x += xorg;
	 prect->y += yorg;
	 prect++;
      }
   }
   prect = prectInit;

   numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
   if (numRects > NUM_STACK_RECTS) {
      pboxClippedBase = (BoxPtr) ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
      if (!pboxClippedBase)
	 return;
   } else
      pboxClippedBase = stackRects;


   pboxClipped = pboxClippedBase;

   if (REGION_NUM_RECTS(prgnClip) == 1) {
      int   x1, y1, x2, y2, bx2, by2;

      pextent = REGION_RECTS(prgnClip);
      x1 = pextent->x1;
      y1 = pextent->y1;
      x2 = pextent->x2;
      y2 = pextent->y2;
      while (nrectFill--) {
	 if ((pboxClipped->x1 = prect->x) < x1)
	    pboxClipped->x1 = x1;

	 if ((pboxClipped->y1 = prect->y) < y1)
	    pboxClipped->y1 = y1;

	 bx2 = (int)prect->x + (int)prect->width;
	 if (bx2 > x2)
	    bx2 = x2;
	 pboxClipped->x2 = bx2;

	 by2 = (int)prect->y + (int)prect->height;
	 if (by2 > y2)
	    by2 = y2;
	 pboxClipped->y2 = by2;

	 prect++;
	 if ((pboxClipped->x1 < pboxClipped->x2) &&
	     (pboxClipped->y1 < pboxClipped->y2)) {
	    pboxClipped++;
	 }
      }
   } else {
      int   x1, y1, x2, y2, bx2, by2;

      pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
      x1 = pextent->x1;
      y1 = pextent->y1;
      x2 = pextent->x2;
      y2 = pextent->y2;
      while (nrectFill--) {
	 BoxRec box;

	 if ((box.x1 = prect->x) < x1)
	    box.x1 = x1;

	 if ((box.y1 = prect->y) < y1)
	    box.y1 = y1;

	 bx2 = (int)prect->x + (int)prect->width;
	 if (bx2 > x2)
	    bx2 = x2;
	 box.x2 = bx2;

	 by2 = (int)prect->y + (int)prect->height;
	 if (by2 > y2)
	    by2 = y2;
	 box.y2 = by2;

	 prect++;

	 if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    continue;

	 n = REGION_NUM_RECTS(prgnClip);
	 pbox = REGION_RECTS(prgnClip);

       /*
        * clip the rectangle to each box in the clip region this is logically
        * equivalent to calling Intersect()
        */
	 while (n--) {
	    pboxClipped->x1 = max(box.x1, pbox->x1);
	    pboxClipped->y1 = max(box.y1, pbox->y1);
	    pboxClipped->x2 = min(box.x2, pbox->x2);
	    pboxClipped->y2 = min(box.y2, pbox->y2);
	    pbox++;

	  /* see if clipping left anything */
	    if (pboxClipped->x1 < pboxClipped->x2 &&
		pboxClipped->y1 < pboxClipped->y2) {
	       pboxClipped++;
	    }
	 }
      }
   }


   if (pboxClipped != pboxClippedBase) {
      n = pboxClipped - pboxClippedBase;
      switch (pGC->fillStyle) {
	case FillSolid:
	   BLOCK_CURSOR;
	   WaitQueue16_32(3,5);PCI_HACK();
	   S3_OUTW32(FRGD_COLOR, (pGC->fgPixel));
	   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
	   S3_OUTW32(WRT_MASK, pGC->planemask);

	   pboxClipped = pboxClippedBase;
	   while (n--) {
	      WaitQueue(5);PCI_HACK();
	      S3_OUTW(CUR_X, (short)(pboxClipped->x1));
	      S3_OUTW(CUR_Y, (short)(pboxClipped->y1));
	      S3_OUTW(MAJ_AXIS_PCNT,
		    (short)(pboxClipped->x2 - pboxClipped->x1 - 1));
	      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT |
		    (short)(pboxClipped->y2 - pboxClipped->y1 - 1));
	      S3_OUTW(CMD, CMD_RECT | INC_Y | INC_X | DRAW | PLANAR | WRTDATA);

	      pboxClipped++;
	   }

	   WaitQueue(2);
           WaitIdle();
	   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
	   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
	   UNBLOCK_CURSOR;
	   break;

	case FillTiled:
	   xrot = pDrawable->x + pGC->patOrg.x;
	   yrot = pDrawable->y + pGC->patOrg.y;

	   pPix = pGC->tile.pixmap;
	   width = pPix->drawable.width;
	   height = pPix->drawable.height;
	   pixWidth = PixmapBytePad(width, pPix->drawable.depth);

	   pboxClipped = pboxClippedBase;
	   if (pci = DoCacheTile(pPix, inf)) {
	      while (n--) {
		 int w, h;
		 w = pboxClipped->x2 - pboxClipped->x1;
		 h = pboxClipped->y2 - pboxClipped->y1;
		 amigaCVCImageFill(pci, pboxClipped->x1, pboxClipped->y1,
			      w, h,
			      xrot, yrot,
			      s3alu[pGC->alu], pGC->planemask, inf);
		 pboxClipped++;
	      }
	   } else {
	      while (n--) {
		 amigaCVImageFill (pboxClipped->x1, pboxClipped->y1,
				    pboxClipped->x2 - pboxClipped->x1,
				    pboxClipped->y2 - pboxClipped->y1,
				    pPix->devPrivate.ptr, pixWidth,
				    width, height, xrot, yrot,
				    s3alu[pGC->alu], pGC->planemask, inf);
		 pboxClipped++;
	      }
	   }
	   break;
	case FillStippled:
	   xrot = pDrawable->x + pGC->patOrg.x;
	   yrot = pDrawable->y + pGC->patOrg.y;

	   pPix = pGC->stipple;
	   width = pPix->drawable.width;
	   height = pPix->drawable.height;
	   pixWidth = PixmapBytePad(width, pPix->drawable.depth);

	   pboxClipped = pboxClippedBase;
	   if (pci = DoCacheOpStipple(pPix, inf)) {
	      while (n--) {
	         int w, h;
	         w = pboxClipped->x2 - pboxClipped->x1;
		 h = pboxClipped->y2 - pboxClipped->y1;
	       	 amigaCVCImageStipple(pci, pboxClipped->x1, pboxClipped->y1,
				 w, h,
				 xrot, yrot, pGC->fgPixel,
				 s3alu[pGC->alu], pGC->planemask, inf);
		 pboxClipped++;
	      }
	   } else {
	      while (n--) {
		 amigaCVImageStipple(pboxClipped->x1, pboxClipped->y1,
				pboxClipped->x2 - pboxClipped->x1,
				pboxClipped->y2 - pboxClipped->y1,
				pPix->devPrivate.ptr, pixWidth,
				width, height, xrot, yrot,
				pGC->fgPixel,
				s3alu[pGC->alu], pGC->planemask, inf);
		 pboxClipped++;
	      }
	   }
	   break;
	case FillOpaqueStippled:
	   xrot = pDrawable->x + pGC->patOrg.x;
	   yrot = pDrawable->y + pGC->patOrg.y;

	   pPix = pGC->stipple;
	   width = pPix->drawable.width;
	   height = pPix->drawable.height;
	   pixWidth = PixmapBytePad(width, pPix->drawable.depth);

	   pboxClipped = pboxClippedBase;

	   if (pci = DoCacheOpStipple(pPix, inf)) {
	      while (n--) {
      	         int w, h;
	         w = pboxClipped->x2 - pboxClipped->x1;
		 h = pboxClipped->y2 - pboxClipped->y1;
		 amigaCVCImageOpStipple(pci, pboxClipped->x1, pboxClipped->y1,
				   w, h,
				   xrot, yrot,
				   pGC->fgPixel, pGC->bgPixel,
				   s3alu[pGC->alu],
				   pGC->planemask, inf);
		 pboxClipped++;
	      }
	   } else {
	      while (n--) {
		 amigaCVImageOpStipple(pboxClipped->x1, pboxClipped->y1,
				  pboxClipped->x2 - pboxClipped->x1,
				  pboxClipped->y2 - pboxClipped->y1,
				  pPix->devPrivate.ptr, pixWidth,
				  width, height, xrot, yrot,
				  pGC->fgPixel, pGC->bgPixel,
				  s3alu[pGC->alu],
				  pGC->planemask, inf);
		 pboxClipped++;
	      }
	   }
	   break;


      }
   }
   if (pboxClippedBase != stackRects)
      DEALLOCATE_LOCAL(pboxClippedBase);

}


void
amigaCVFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
     DrawablePtr pDrawable;
     GCPtr pGC;
     int   nInit;		/* number of spans to fill */
     DDXPointPtr pptInit;	/* pointer to list of start points */
     int  *pwidthInit;		/* pointer to list of n widths */
     int   fSorted;
{
   int   n;			/* number of spans to fill */
   register DDXPointPtr ppt;	/* pointer to list of start points */
   register int *pwidth;	/* pointer to list of n widths */
   DDXPointPtr initPpt;
   int *initPwidth;
   fbFd *inf = amigaInfo(pDrawable->pScreen);
   volatile caddr_t vgaBase = (inf->regs);

   if (pDrawable->type != DRAWABLE_WINDOW) {
      switch (pDrawable->bitsPerPixel) {
	case 1:
	   __dolog("should call mfbSolidFillSpans\n");
	   break;
	case 8:
        case 16:
        case 32:
	   __dolog("should call cfbSolidFillSpans\n");
	   break;
	default:
	   ErrorF("Unsupported pixmap depth\n");
	   break;
      }
      return;
   }
   if (!(pGC->planemask))
      return;

   n = nInit * miFindMaxBand(cfbGetCompositeClip(pGC));
   initPwidth = pwidth = (int *)ALLOCATE_LOCAL(n * sizeof(int));

   initPpt = ppt = (DDXPointRec *) ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
   if (!ppt || !pwidth) {
      if (ppt)
	 DEALLOCATE_LOCAL(ppt);
      if (pwidth)
	 DEALLOCATE_LOCAL(pwidth);
      return;
   }
   n = miClipSpans(cfbGetCompositeClip(pGC),
		   pptInit, pwidthInit, nInit,
		   ppt, pwidth, fSorted);

   BLOCK_CURSOR;
   WaitQueue16_32(3,5); PCI_HACK();
   S3_OUTW32(FRGD_COLOR, (pGC->fgPixel));
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
   S3_OUTW32(WRT_MASK, pGC->planemask);

   while (n--) {
      WaitQueue(5);PCI_HACK();
      S3_OUTW(CUR_X, ppt->x);
      S3_OUTW(CUR_Y, ppt->y);
      S3_OUTW(MAJ_AXIS_PCNT, ((short)*pwidth) - 1);
      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | 0);
      S3_OUTW(CMD, CMD_RECT | INC_Y | INC_X | DRAW | PLANAR | WRTDATA);

      ppt++;
      pwidth++;
   }

   WaitQueue(2); 
   WaitIdle();
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;

   DEALLOCATE_LOCAL(initPpt);
   DEALLOCATE_LOCAL(initPwidth);

   __dolog("AmigaCVFillSpans2");
}

