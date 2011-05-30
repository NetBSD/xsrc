/* $XConsortium: s3bstor.c,v 1.3 95/01/16 13:16:51 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3bstor.c,v 3.7 1995/01/28 17:01:49 dawes Exp $ */
/*-
 * s3bstore.c --
 *	Functions required by the backing-store implementation in MI.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
 *
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: s3bstor.c,v 2.2 1993/06/22 20:54:09 jon Exp jon
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#if 0
#include    "amiga.h"
#include    "cfb.h"
#if 0
#endif
#include    "X.h"
#include    "mibstore.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"
#include    "amigaCV.h"
#endif
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
#include        "cfb16.h"
#include        "cfb32.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"amigaCV.h"
#include	"migc.h"

/* Externs from amigaCV.c */

extern short s3alu[];



void
amigaCVSaveAreas(pPixmap, prgnSave, xorg, yorg, pWin)
     PixmapPtr pPixmap;		/* Backing pixmap */
     RegionPtr prgnSave;	/* Region to save (pixmap-relative) */
     int   xorg;		/* X origin of region */
     int   yorg;		/* Y origin of region */
     WindowPtr pWin;
{
   register BoxPtr pBox;
   register int i;
   int   pixWidth;
   fbFd *inf = amigaInfo(amigaCVsavepScreen);
   volatile caddr_t vgaBase =  (inf->regs);

   WaitIdle();
   /* cfb still faster than our Image read :-( */
  	switch (pPixmap->drawable.bitsPerPixel) {
	case 8:
	    cfbSaveAreas(pPixmap, prgnSave, xorg, yorg, pWin);
	    return;
	case 15: case 16:
	    cfb16SaveAreas(pPixmap, prgnSave, xorg, yorg, pWin);
	    return;
	case 24: case 32:
	    cfb32SaveAreas(pPixmap, prgnSave, xorg, yorg, pWin);
	    return;
	}
#if 0
   i = REGION_NUM_RECTS(prgnSave);
   pBox = REGION_RECTS(prgnSave);

   pixWidth = PixmapBytePad(pPixmap->drawable.width, pPixmap->drawable.depth);

   while (i--) {
      amigaCVImageRead (pBox->x1 + xorg, pBox->y1 + yorg,
			 pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			 pPixmap->devPrivate.ptr, pixWidth,
			 pBox->x1, pBox->y1, ~0, inf);
      pBox++;
   }
#endif
}

void
amigaCVRestoreAreas(pPixmap, prgnRestore, xorg, yorg, pWin)
     PixmapPtr pPixmap;		/* Backing pixmap */
     RegionPtr prgnRestore;	/* Region to restore (screen-relative) */
     int   xorg;		/* X origin of window */
     int   yorg;		/* Y origin of window */
     WindowPtr pWin;
{
   register BoxPtr pBox;
   register int i;
   int   pixWidth;
   fbFd *inf = amigaInfo(amigaCVsavepScreen);

#if 0 /* Writing we are faster... */
   if (!xf86VTSema)
   {
	switch (pPixmap->drawable.bitsPerPixel) {
	case 8:
	    cfbRestoreAreas(pPixmap, prgnRestore, xorg, yorg, pWin);
	    return;
	case 16:
	    cfb16RestoreAreas(pPixmap, prgnRestore, xorg, yorg, pWin);
	    return;
	 case 32:
	    cfb32RestoreAreas(pPixmap, prgnRestore, xorg, yorg, pWin);
	    return;
	}
   }
#endif 

   i = REGION_NUM_RECTS(prgnRestore);
   pBox = REGION_RECTS(prgnRestore);

   pixWidth = PixmapBytePad(pPixmap->drawable.width, pPixmap->drawable.depth);

   while (i--) {
      amigaCVImageWrite (pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pPixmap->devPrivate.ptr, pixWidth,
			  pBox->x1 - xorg, pBox->y1 - yorg,
			  s3alu[GXcopy], ~0, inf);
      pBox++;
   }
}
