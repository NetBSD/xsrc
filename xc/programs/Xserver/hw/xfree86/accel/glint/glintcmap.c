/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/glintcmap.c,v 1.5.2.1 1998/07/30 06:23:43 hohndel Exp $ */
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

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Adapted to the I128 chipset by Robin Cutshaw
 *
 * Adapted for the 3DLabs GLINT by Alan Hourihane <alanh@fairlite.demon.co.uk>
 * and Stefan Dirsch <sndirsch@suse.de>
 */

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "windowstr.h"
#include "compiler.h"

#include "glint_regs.h"
#include "glint.h"

#include "glintcurs.h"
#define GLINT_SERVER
#include "IBMRGB.h"

#include "xf86_Config.h"

#ifdef XFreeXDGA
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

extern Bool UsePCIRetry;
extern int pciaddr;
extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
extern struct glintmem glintmem;
extern int coprotype;

#define NOMAPYET        (ColormapPtr) 0

ColormapPtr InstalledMaps[MAXSCREENS]; /* current colormap for each screen */

LUTENTRY currentglintdac[256];

int
glintListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps)
{

 /*
  * By the time we are processing requests, we can guarantee that there is
  * always a colormap installed
  */

   *pmaps = InstalledMaps[pScreen->myNum]->mid;
   return (1);
}

void
glintRestoreDACvalues()
{
   int i;

   if (xf86VTSema) {
     BLOCK_CURSOR;
	if (IS_3DLABS_PM2_CLASS(coprotype)) {
		GLINT_SLOW_WRITE_REG(0x00, PM2DACWriteAddress);
	} else {
      		GLINT_SLOW_WRITE_REG(0x00, IBMRGB_WRITE_ADDR);
	}
      
      for (i=0; i<256; i++) {
	if (IS_3DLABS_PM2_CLASS(coprotype)) {
	GLINT_SLOW_WRITE_REG(currentglintdac[i].r, PM2DACData);
	GLINT_SLOW_WRITE_REG(currentglintdac[i].g, PM2DACData);
	GLINT_SLOW_WRITE_REG(currentglintdac[i].b, PM2DACData);
	} else {
	GLINT_SLOW_WRITE_REG(currentglintdac[i].r, IBMRGB_RAMDAC_DATA);
	GLINT_SLOW_WRITE_REG(currentglintdac[i].g, IBMRGB_RAMDAC_DATA);
	GLINT_SLOW_WRITE_REG(currentglintdac[i].b, IBMRGB_RAMDAC_DATA);
	}
      }
      UNBLOCK_CURSOR;
   }
}

int
glintGetInstalledColormaps(ScreenPtr pScreen, ColormapPtr *pmap)
{
  *pmap = InstalledMaps[pScreen->myNum];
  return(1);
}


void
glintStoreColors( ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
   int   i;
   xColorItem directDefs[256];

   if (pmap != InstalledMaps[pmap->pScreen->myNum])
      return;

   if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
      ndef = cfbExpandDirectColors(pmap, ndef, pdefs, directDefs);
      pdefs = directDefs;
   }
   BLOCK_CURSOR;
   for (i = 0; i < ndef; i++) {
      unsigned char r, g, b;

         r = currentglintdac[pdefs[i].pixel].r =
	    xf86rGammaMap[pdefs[i].red   >> 8];
         g = currentglintdac[pdefs[i].pixel].g =
	    xf86gGammaMap[pdefs[i].green >> 8];
         b = currentglintdac[pdefs[i].pixel].b =
	    xf86bGammaMap[pdefs[i].blue  >> 8];

      if (xf86VTSema
#ifdef XFreeXDGA
		|| ((glintInfoRec.directMode & XF86DGADirectGraphics)
		&& !(glintInfoRec.directMode & XF86DGADirectColormap))
		|| (glintInfoRec.directMode & XF86DGAHasColormap)
#endif
	 ) {
	if (IS_3DLABS_PM2_CLASS(coprotype)) {
	 GLINT_SLOW_WRITE_REG(pdefs[i].pixel, PM2DACWriteAddress); 
	 GLINT_SLOW_WRITE_REG(r, PM2DACData);
	 GLINT_SLOW_WRITE_REG(g, PM2DACData);
	 GLINT_SLOW_WRITE_REG(b, PM2DACData); 
	} else {
	 GLINT_SLOW_WRITE_REG(pdefs[i].pixel, IBMRGB_WRITE_ADDR); 
	 GLINT_SLOW_WRITE_REG(r, IBMRGB_RAMDAC_DATA);
	 GLINT_SLOW_WRITE_REG(g, IBMRGB_RAMDAC_DATA);
	 GLINT_SLOW_WRITE_REG(b, IBMRGB_RAMDAC_DATA); 
	}
      }
   }
   UNBLOCK_CURSOR;
}

void
glintInstallColormap(ColormapPtr pmap)
{
   ColormapPtr oldmap = InstalledMaps[pmap->pScreen->myNum];
   int   entries;
   Pixel *ppix;
   xrgb *prgb;
   xColorItem *defs;
   int   i;

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

   QueryColors(pmap, entries, ppix, prgb);

   for (i = 0; i < entries; i++) {	/* convert xrgbs to xColorItems */
      defs[i].pixel = ppix[i];
      defs[i].red = prgb[i].red;
      defs[i].green = prgb[i].green;
      defs[i].blue = prgb[i].blue;
      defs[i].flags = DoRed | DoGreen | DoBlue;
   }

   glintStoreColors(pmap, entries, defs);

   WalkTree(pmap->pScreen, TellGainedMap, &pmap->mid);
#ifdef WORKWORKWORK
   glintRenewCursorColor(pmap->pScreen);
#endif
   DEALLOCATE_LOCAL(ppix);
   DEALLOCATE_LOCAL(prgb);
   DEALLOCATE_LOCAL(defs);
}

void
glintUninstallColormap(ColormapPtr pmap)
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
glintRestoreColor0(ScreenPtr pScreen)
{
   Pixel pix = 0;
   xrgb  rgb;
   
   if (InstalledMaps[pScreen->myNum] == NOMAPYET)
      return;

   QueryColors(InstalledMaps[pScreen->myNum], 1, &pix, &rgb);

   BLOCK_CURSOR;
   if (IS_3DLABS_PM2_CLASS(coprotype)) {
   GLINT_SLOW_WRITE_REG(0x00, PM2DACWriteAddress);

   GLINT_SLOW_WRITE_REG(xf86rGammaMap[rgb.red >> 8], PM2DACData);
   GLINT_SLOW_WRITE_REG(xf86gGammaMap[rgb.green >> 8], PM2DACData);
   GLINT_SLOW_WRITE_REG(xf86bGammaMap[rgb.blue >> 8], PM2DACData);
   } else {
   GLINT_SLOW_WRITE_REG(0x00, IBMRGB_WRITE_ADDR);

   GLINT_SLOW_WRITE_REG(xf86rGammaMap[rgb.red >> 8], IBMRGB_RAMDAC_DATA);
   GLINT_SLOW_WRITE_REG(xf86gGammaMap[rgb.green >> 8], IBMRGB_RAMDAC_DATA);
   GLINT_SLOW_WRITE_REG(xf86bGammaMap[rgb.blue >> 8], IBMRGB_RAMDAC_DATA);
   }

   UNBLOCK_CURSOR;
}

void
glintUnblankScreen(ScreenPtr pScreen)
{
}

void
glintBlankScreen(ScreenPtr pScreen)
{
}
