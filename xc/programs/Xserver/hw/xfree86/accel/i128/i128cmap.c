/* $TOG: i128cmap.c /main/4 1997/10/19 15:03:02 kaleb $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Thomas Roell makes no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * THOMAS ROELL AND KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL THOMAS ROELL OR KEVIN E. MARTIN BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * Rewritten for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
 * 
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/i128/i128cmap.c,v 3.2.2.2 2000/02/15 23:36:26 robin Exp $ */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: i128cmap.c,v 2.2 1993/06/22 20:54:09 jon Exp jon
 *
 * Adapted to the I128 chipset by Robin Cutshaw
 */


#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "windowstr.h"
#include "compiler.h"

#ifdef XFreeXDGA    
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif      

#include "i128.h"
#include "i128reg.h"
#include "i128Cursor.h"

extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
extern struct i128mem i128mem;

#define NOMAPYET        (ColormapPtr) 0

static ColormapPtr InstalledMaps[MAXSCREENS];

/* current colormap for each screen */

LUTENTRY currenti128dac[256];

int
i128ListInstalledColormaps(pScreen, pmaps)
     ScreenPtr pScreen;
     Colormap *pmaps;
{

 /*
  * By the time we are processing requests, we can guarantee that there is
  * always a colormap installed
  */

   *pmaps = InstalledMaps[pScreen->myNum]->mid;
   return (1);
}

void
i128RestoreDACvalues()
{
   int i;

   if (xf86VTSema) {
      BLOCK_CURSOR;
      i128mem.rbase_g[WR_ADR] = 0x00;					MB;

      for (i=0; i < 256; i++) {
	 i128mem.rbase_g[PAL_DAT] = currenti128dac[i].r;		MB;
	 i128mem.rbase_g[PAL_DAT] = currenti128dac[i].g;		MB;
	 i128mem.rbase_g[PAL_DAT] = currenti128dac[i].b;		MB;
      }
      UNBLOCK_CURSOR;
   }
}

int
i128GetInstalledColormaps(pScreen, pmap)
     ScreenPtr        pScreen;
     ColormapPtr      *pmap;
{
  *pmap = InstalledMaps[pScreen->myNum];
  return(1);
}


void
i128StoreColors(pmap, ndef, pdefs)
     ColormapPtr pmap;
     int   ndef;
     xColorItem *pdefs;
{
   int   i;
   xColorItem directDefs[256];
   extern Bool i128DAC8Bit;

   if (pmap != InstalledMaps[pmap->pScreen->myNum])
      return;

   if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
      ndef = cfbExpandDirectColors(pmap, ndef, pdefs, directDefs);
      pdefs = directDefs;
   }
   BLOCK_CURSOR;
   for (i = 0; i < ndef; i++) {
      unsigned char r, g, b;

      if (i128DAC8Bit) {
         r = currenti128dac[pdefs[i].pixel].r =
	    xf86rGammaMap[pdefs[i].red   >> 8];
         g = currenti128dac[pdefs[i].pixel].g =
	    xf86gGammaMap[pdefs[i].green >> 8];
         b = currenti128dac[pdefs[i].pixel].b =
	    xf86bGammaMap[pdefs[i].blue  >> 8];
      } else {
         r = currenti128dac[pdefs[i].pixel].r =
	    xf86rGammaMap[pdefs[i].red   >> 8] >> 2;
         g = currenti128dac[pdefs[i].pixel].g =
	    xf86gGammaMap[pdefs[i].green >> 8] >> 2;
         b = currenti128dac[pdefs[i].pixel].b =
	    xf86bGammaMap[pdefs[i].blue  >> 8] >> 2;
      }
      if (xf86VTSema
#ifdef XFreeXDGA    
            || ((i128InfoRec.directMode & XF86DGADirectGraphics)
                && !(i128InfoRec.directMode & XF86DGADirectColormap))
            || (i128InfoRec.directMode & XF86DGAHasColormap)
#endif
         )
      {
	 i128mem.rbase_g[WR_ADR] = pdefs[i].pixel;			MB;
	 i128mem.rbase_g[PAL_DAT] = r;					MB;
	 i128mem.rbase_g[PAL_DAT] = g;					MB;
	 i128mem.rbase_g[PAL_DAT] = b;					MB;
      }
   }
   UNBLOCK_CURSOR;
}

void
i128InstallColormap(pmap)
     ColormapPtr pmap;
{
   ColormapPtr oldmap = InstalledMaps[pmap->pScreen->myNum];
   int   entries;
   Pixel *ppix;
   xrgb *prgb;
   xColorItem *defs;
   int   i,j;

   if (pmap == oldmap)
      return;

   if ((pmap->pVisual->class | DynamicClass) == DirectColor)
      entries = (pmap->pVisual->redMask |
		 pmap->pVisual->greenMask |
		 pmap->pVisual->blueMask) + 1;
   else
      entries = pmap->pVisual->ColormapEntries;

   ppix = (Pixel *) ALLOCATE_LOCAL(entries * sizeof(Pixel));
   prgb = (xrgb *) ALLOCATE_LOCAL(entries * sizeof(xrgb));
   defs = (xColorItem *) ALLOCATE_LOCAL(entries * sizeof(xColorItem));

   if (oldmap != NOMAPYET)
      WalkTree(pmap->pScreen, TellLostMap, &oldmap->mid);

   InstalledMaps[pmap->pScreen->myNum] = pmap;

   for (i = 0; i < entries; i++)
      ppix[i] = i;

  if (pmap->class == GrayScale || pmap->class == PseudoColor)
    {
      for ( i=j=0; i<entries; i++) 
        {
	  if (pmap->red[i].fShared || pmap->red[i].refcnt != 0)
	    {
	      defs[j].pixel = i;
              defs[j].flags = DoRed|DoGreen|DoBlue;
	      if (pmap->red[i].fShared)
	        {
	          defs[j].red = pmap->red[i].co.shco.red->color;
	          defs[j].green = pmap->red[i].co.shco.green->color;
	          defs[j].blue = pmap->red[i].co.shco.blue->color;
	        }
	        else if (pmap->red[i].refcnt != 0)
	        {
	          defs[j].red = pmap->red[i].co.local.red;
	          defs[j].green = pmap->red[i].co.local.green;
	          defs[j].blue = pmap->red[i].co.local.blue;
	        }
	      j++;
	    }
        }
      entries = j;
    }
  else
    {
      QueryColors( pmap, entries, ppix, prgb);

      for ( i=0; i<entries; i++) /* convert xrgbs to xColorItems */
        {
          defs[i].pixel = ppix[i];
          defs[i].red = prgb[i].red;
          defs[i].green = prgb[i].green;
          defs[i].blue = prgb[i].blue;
          defs[i].flags =  DoRed|DoGreen|DoBlue;
        }
    }

   i128StoreColors(pmap, entries, defs);

   WalkTree(pmap->pScreen, TellGainedMap, &pmap->mid);
#ifdef WORKWORKWORK
   i128RenewCursorColor(pmap->pScreen);
#endif
   DEALLOCATE_LOCAL(ppix);
   DEALLOCATE_LOCAL(prgb);
   DEALLOCATE_LOCAL(defs);
}

void
i128UninstallColormap(pmap)
     ColormapPtr pmap;
{
   ColormapPtr defColormap;

   if (pmap != InstalledMaps[pmap->pScreen->myNum])
      return;

   defColormap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
					      RT_COLORMAP);

   if (defColormap == InstalledMaps[pmap->pScreen->myNum])
      return;

   (*pmap->pScreen->InstallColormap) (defColormap);
}

/* This is for the screen saver */
void
i128RestoreColor0(pScreen)
     ScreenPtr pScreen;
{
   Pixel pix = 0;
   xrgb  rgb;
   extern Bool i128DAC8Bit;
   
   if (InstalledMaps[pScreen->myNum] == NOMAPYET)
      return;

   QueryColors(InstalledMaps[pScreen->myNum], 1, &pix, &rgb);

   BLOCK_CURSOR;
   i128mem.rbase_g[WR_ADR] = 0x00;					MB;
   if (i128DAC8Bit) {
      i128mem.rbase_g[PAL_DAT] = xf86rGammaMap[rgb.red   >> 8];		MB;
      i128mem.rbase_g[PAL_DAT] = xf86gGammaMap[rgb.green >> 8];		MB;
      i128mem.rbase_g[PAL_DAT] = xf86bGammaMap[rgb.blue  >> 8];		MB;
   } else {
      i128mem.rbase_g[PAL_DAT] = xf86rGammaMap[rgb.red   >> 8] >> 2;	MB;
      i128mem.rbase_g[PAL_DAT] = xf86gGammaMap[rgb.green >> 8] >> 2;	MB;
      i128mem.rbase_g[PAL_DAT] = xf86bGammaMap[rgb.blue  >> 8] >> 2;	MB;
   }
   UNBLOCK_CURSOR;
}
