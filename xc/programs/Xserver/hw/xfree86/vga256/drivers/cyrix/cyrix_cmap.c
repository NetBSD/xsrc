/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_cmap.c,v 1.1.2.1 1998/09/13 12:29:09 hohndel Exp $ */
/*
 * Copyright 1998 by Annius Groenink, Amsterdam.
 *
 * Derived from vgaCmap.c from X332 (Thomas Roell).
 *
 * Large bits of vgaCmap.c are copied here.  I suspect large bits of
 * duplicate code can be removed, tried that, but it didn't work.
 * Then again, I linked this X332 cmap.c against the 331 link kit...
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Annius Groenink not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Annius Groenink makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ANNIUS GROENINK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: $ */

#include "X.h"
#include "Xproto.h"
#include "windowstr.h"
#include "compiler.h"

#include "xf86.h"
#include "vga.h"

#ifdef XFreeXDGA
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#include "cyrix.h"


#define NOMAPYET        (ColormapPtr) 0

#if 0
extern Bool clgd6225Lcd;
#endif

static ColormapPtr InstalledMaps[MAXSCREENS];
				/* current colormap for each screen */

int
CYRIXListInstalledColormaps(pScreen, pmaps)
     ScreenPtr	pScreen;
     Colormap	*pmaps;
{
  /* By the time we are processing requests, we can guarantee that there
   * is always a colormap installed */
  
  *pmaps = InstalledMaps[pScreen->myNum]->mid;
  return(1);
}

#if 0
/* there is no place too hook this function; let's hope
   vgaGetInstaledColormaps is not called, because it will
   return nonsense (all other vgaCmap functions are bypassed). */
int
CYRIXGetInstalledColormaps(pScreen, pmaps)
     ScreenPtr		pScreen;
     ColormapPtr	*pmaps;
{
  /* By the time we are processing requests, we can guarantee that there
   * is always a colormap installed */
  
  *pmaps = InstalledMaps[pScreen->myNum];
  return(1);
}
#endif

static int cyrixCheckColorMap(ColormapPtr pmap)
{
  return (pmap != InstalledMaps[pmap->pScreen->myNum]);
}


void
CYRIXStoreColors(pmap, ndef, pdefs)
     ColormapPtr	pmap;
     int		ndef;
     xColorItem	        *pdefs;
{
    int		i;
    unsigned char *cmap, *tmp;
    xColorItem	directDefs[256];
    Bool          new_overscan = FALSE;
    unsigned char overscan = ((vgaHWPtr)vgaNewVideoState)->Attribute[OVERSCAN];
    unsigned char tmp_overscan;

    if (cyrixCheckColorMap(pmap))
        return;

    /* GJA -- We don't want cfb code right now (in vga16 server) */
    /* avg: code not triggered */
    if ((pmap->pVisual->class | DynamicClass) == DirectColor)
    {
        ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
        pdefs = directDefs;
    }

    GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;

    for(i = 0; i < ndef; i++)
    {
        if (pdefs[i].pixel == overscan)
	{
	    new_overscan = TRUE;
	}
        cmap = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
        cmap[0] = pdefs[i].red   >> 10;
        cmap[1] = pdefs[i].green >> 10;
        cmap[2] = pdefs[i].blue  >> 10;

        if (xf86VTSema
#ifdef XFreeXDGA
	    || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
	        && !(vga256InfoRec.directMode & XF86DGADirectColormap))
	    || (vga256InfoRec.directMode & XF86DGAHasColormap)
#endif
	   )
	{
	    GX_REG(DC_PAL_ADDRESS) = pdefs[i].pixel;
	    GX_REG(DC_PAL_DATA) = (cmap[0] << 12)
				| (cmap[1] << 6)
				| (cmap[2]);
	}
    }	
#if 0

    if (new_overscan)
    {
	new_overscan = FALSE;
        for(i = 0; i < ndef; i++)
        {
            if (pdefs[i].pixel == overscan)
	    {
	        if ((pdefs[i].red != 0) || 
	            (pdefs[i].green != 0) || 
	            (pdefs[i].blue != 0))
	        {
	            new_overscan = TRUE;
		    tmp_overscan = overscan;
        	    tmp = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
	        }
	        break;
	    }
        }
        if (new_overscan)
        {
            /*
             * Find a black pixel, or the nearest match.
             */
            for (i=255; i >= 0; i--)
	    {
                cmap = &((vgaHWPtr)vgaNewVideoState)->DAC[i*3];
	        if ((cmap[0] == 0) && (cmap[1] == 0) && (cmap[2] == 0))
	        {
	            overscan = i;
	            break;
	        }
	        else
	        {
	            if ((cmap[0] < tmp[0]) && 
		        (cmap[1] < tmp[1]) && (cmap[2] < tmp[2]))
	            {
		        tmp = cmap;
		        tmp_overscan = i;
	            }
	        }
	    }
	    if (i < 0)
	    {
	        overscan = tmp_overscan;
	    }
	    ((vgaHWPtr)vgaNewVideoState)->Attribute[OVERSCAN] = overscan;
            if (xf86VTSema
#ifdef XFreeXDGA
	        || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
	            && !(vga256InfoRec.directMode & XF86DGADirectColormap))
	        || (vga256InfoRec.directMode&XF86DGAHasColormap)
#endif
	       )
	    {
#ifndef PC98_EGC
	        (void)inb(vgaIOBase + 0x0A);
	        outb(0x3C0, OVERSCAN);
	        outb(0x3C0, overscan);
	        (void)inb(vgaIOBase + 0x0A);
	        outb(0x3C0, 0x20);
#endif
	    }
        }
    }
#endif

    GX_REG(DC_UNLOCK) = 0;
}


/* it is sad that we have to literally copy this function,
   but there doesn't seem to be an alternative that works.
   (the reason turns out to be that vgaCmap.c in 3.3.1 calls
   vgaStoreColors, not pmap->pScreen->StoreColors) */
void
CYRIXInstallColormap(pmap)
     ColormapPtr	pmap;
{ ColormapPtr oldmap = InstalledMaps[pmap->pScreen->myNum];
  int         entries;
  Pixel *     ppix;
  xrgb *      prgb;
  xColorItem *defs;
  int         i;

  if (pmap == oldmap)
    return;

  if ((pmap->pVisual->class | DynamicClass) == DirectColor)
    entries = (pmap->pVisual->redMask |
	       pmap->pVisual->greenMask |
	       pmap->pVisual->blueMask) + 1;
  else
    entries = pmap->pVisual->ColormapEntries;

  ppix = (Pixel *)ALLOCATE_LOCAL( entries * sizeof(Pixel));
  prgb = (xrgb *)ALLOCATE_LOCAL( entries * sizeof(xrgb));
  defs = (xColorItem *)ALLOCATE_LOCAL(entries * sizeof(xColorItem));

  if ( oldmap != NOMAPYET)
    WalkTree( pmap->pScreen, TellLostMap, &oldmap->mid);

  InstalledMaps[pmap->pScreen->myNum] = pmap;

  for ( i=0; i<entries; i++) ppix[i] = i;

  QueryColors( pmap, entries, ppix, prgb);

  for ( i=0; i<entries; i++) /* convert xrgbs to xColorItems */
    {
      defs[i].pixel = ppix[i];
      defs[i].red = prgb[i].red;
      defs[i].green = prgb[i].green;
      defs[i].blue = prgb[i].blue;
      defs[i].flags =  DoRed|DoGreen|DoBlue;
    }

  pmap->pScreen->StoreColors( pmap, entries, defs);

  WalkTree(pmap->pScreen, TellGainedMap, &pmap->mid);
  
  DEALLOCATE_LOCAL(ppix);
  DEALLOCATE_LOCAL(prgb);
  DEALLOCATE_LOCAL(defs);
}

void
CYRIXUninstallColormap(pmap)
     ColormapPtr pmap;
{
  ColormapPtr defColormap;
  
  if ( pmap != InstalledMaps[pmap->pScreen->myNum] )
    return;

  defColormap = (ColormapPtr) LookupIDByType( pmap->pScreen->defColormap,
					      RT_COLORMAP);

  if (defColormap == InstalledMaps[pmap->pScreen->myNum])
    return;

  (*pmap->pScreen->InstallColormap) (defColormap);
}

