/*
 *Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 *
 *Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 *"Software"), to deal in the Software without restriction, including
 *without limitation the rights to use, copy, modify, merge, publish,
 *distribute, sublicense, and/or sell copies of the Software, and to
 *permit persons to whom the Software is furnished to do so, subject to
 *the following conditions:
 *
 *The above copyright notice and this permission notice shall be
 *included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of the XFree86 Project
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from the XFree86 Project.
 *
 * Authors:	Dakshinamurthy Karra
 *		Suhaib M Siddiqi
 *		Peter Busch
 *		Harold L Hunt II
 */
/* $XFree86: xc/programs/Xserver/hw/xwin/wincmap.c,v 1.4 2001/05/14 16:52:33 alanh Exp $ */

#include "win.h"

/* See Porting Layer Definition - p. 30 */
int
winListInstalledColormapsNativeGDI (ScreenPtr pScreen, Colormap *pmaps)
{
  /* 
   * By the time we are processing requests, we can guarantee that there
   * is always a colormap installed
   */
  
  ErrorF ("\nwinListInstalledColormaps ()\n");
  return miListInstalledColormaps (pScreen, pmaps);
}

/* See Porting Layer Definition - p. 30 */
/* See Programming Windows - p. 663 */
void
winInstallColormapNativeGDI (ColormapPtr pmap)
{
  ErrorF ("\nwinInstallColormap ()\n");
  miInstallColormap (pmap);
}

/* See Porting Layer Definition - p. 30 */
void
winUninstallColormapNativeGDI (ColormapPtr pmap)
{
  ErrorF ("\nwinUninstallColormap ()\n");
  miUninstallColormap (pmap);
}

/* See Porting Layer Definition - p. 30 */
void
winStoreColorsNativeGDI (ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
  ErrorF ("winStoreColors ()\n");
#if 0
  miStoreColors (pmap, ndef, pdefs);
#endif
}

/* See Porting Layer Definition - p. 30 */
void
winResolveColorNativeGDI (unsigned short *pred,
			  unsigned short *pgreen,
			  unsigned short *pblue,
			  VisualPtr	pVisual)
{
  ErrorF ("\nwinResolveColor ()\n");
  miResolveColor (pred, pgreen, pblue, pVisual);
}

/* See Porting Layer Definition - p. 29 */
/* Also refered to as CreateColormap */
Bool
winInitializeColormapNativeGDI (ColormapPtr pmap)
{
  ErrorF ("\nwinInitializeColormap ()\n");
#if 0
  return miInitializeColormap (pmap);
#endif
  return TRUE;
}

int
winExpandDirectColorsNativeGDI (ColormapPtr pmap, int ndef,
				xColorItem *indefs, xColorItem *outdefs)
{
  ErrorF ("\nwinExpandDirectColors ()\n");
  return miExpandDirectColors (pmap, ndef, indefs, outdefs);
}

Bool
winCreateDefColormapNativeGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  Bool			fReturn = TRUE;
  VisualPtr		pVisual;
  ColormapPtr		pcmap = NULL;
  int			i;
  Pixel			pixel;
  unsigned short	nRed, nGreen, nBlue;
  UINT			uiSystemPaletteEntries;
  PALETTEENTRY		*ppeColors = NULL;

  /* Find the root visual */
  for (pVisual = pScreen->visuals;
       pVisual->vid != pScreen->rootVisual;
       pVisual++);

  /*
   *  AllocNone for Dynamic visual classes,
   *  AllocAll for non-Dynamic visual classes.
   */

  /* Allocate an X colormap, owned by client 0 */
  if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &pcmap,
		     AllocNone, 0) != Success)
    {
      ErrorF ("winCreateDefColormapNativeGDI () - CreateColormap failed\n");
      return FALSE;
    }
  if (pcmap == NULL)
    {
      ErrorF ("winCreateDefColormap () - Colormap could not be created\n");
      return FALSE;
    }
  ErrorF ("winCreateDefColormap () - Created a colormap\n");

  /* Get the number of entries in the system palette */
  uiSystemPaletteEntries = GetSystemPaletteEntries (pScreenPriv->hdcScreen,
						    0, 0, NULL);
  ErrorF ("winCreateDefColormap () - uiSystemPaletteEntries %d\n",
	  uiSystemPaletteEntries);
  
  /* Allocate palette entries structure */
  ppeColors = xalloc (uiSystemPaletteEntries * sizeof (PALETTEENTRY));
  
  /* Get system palette entries */
  GetSystemPaletteEntries (pScreenPriv->hdcScreen,
			   0, uiSystemPaletteEntries, ppeColors);

  /* Allocate an X colormap entry for every system palette entry */
  for (i = 0; i < uiSystemPaletteEntries; ++i)
    {
      pixel = i;

      /* Extract the color values for current palette entry */
      nRed = ppeColors[i].peRed << 8;
      nGreen = ppeColors[i].peGreen << 8;
      nBlue = ppeColors[i].peBlue << 8;

#if 0
      ErrorF ("winCreateDefColormap () - Allocating a color: %d; "\
	      "%d %d %d ",
	      pixel, nRed, nGreen, nBlue);
      if (AllocColor (pcmap,
		      &nRed,
		      &nGreen,
		      &nBlue,
		      &pixel,
		      0) != Success)
	break;
#endif

      pcmap->red[i].co.local.red = nRed;
      pcmap->red[i].co.local.green = nGreen;
      pcmap->red[i].co.local.blue = nBlue;
    }

  if (uiSystemPaletteEntries == 0)
    {
      fbInitializeColormap (pcmap);
    }
  else
    {
      /* Set the black and white pixel indices */
      pScreen->whitePixel = uiSystemPaletteEntries - 1;
      pScreen->blackPixel = 0;
    }

  /* Free colormap */
  free (ppeColors);

  /* Install the created colormap */
  (*pScreen->InstallColormap)(pcmap);

  return fReturn;
}

void
winClearVisualTypes (void)
{
#if CYGDEBUG
  ErrorF ("winClearVisualTypes ()\n");
#endif
  miClearVisualTypes ();
}

Bool
winSetVisualTypesNativeGDI (int nDepth, int nVisuals, int nBitsPerRGB)
{
#if CYGDEBUG
  ErrorF ("winSetVisualTypes ()\n");
#endif
  return miSetVisualTypes (nDepth, nVisuals, nBitsPerRGB, -1);
}

Bool
winInitVisualsNativeGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HBITMAP		hbmp;
  BITMAPINFO		*pbmi = xalloc (sizeof (BITMAPINFOHEADER)
					+ 256 * sizeof (RGBQUAD));
  BITMAPV4HEADER	*pbmih = (BITMAPV4HEADER*) pbmi;
  HDC			hdc = GetDC (NULL);

  /* Exit if we could not allocate a bitmapinfo structure */
  if (pbmi == NULL)
    {
      ErrorF ("winInitVisuals () - Could not allocate a "\
	      "bitmapinfo structure\n");
      return FALSE;
    }

  /* Create a bitmap compatible with the primary display */
  hbmp = CreateCompatibleBitmap (hdc, 1, 1);

  ZeroMemory (pbmi, sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD));
  pbmi->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
  
  /* Call GetDIBits for the first time; doesn't do much */
  /*
   * NOTE: This doesn't actually return the bits, because our
   * data pointer is NULL; therefore, we don't have to free
   * memory later.
   */
  GetDIBits (hdc, hbmp,
	     0, 0,
	     NULL,
	     pbmi,
	     0);

#if CYGDEBUG
  ErrorF ("winInitVisuals () - 1st comp %08x rm %08x gm %08x bm %08x\n",
	  pbmi->bmiHeader.biCompression,
	  pbmih->bV4RedMask,
	  pbmih->bV4GreenMask,
	  pbmih->bV4BlueMask);
#endif

  /* Call GetDIBits again if the masks were zero and the color depth > 8 bpp */
  if ((pScreenInfo->dwDepth > 8)
      && (pbmih->bV4RedMask == 0x0
	  || pbmih->bV4GreenMask == 0x0
	  || pbmih->bV4BlueMask == 0x0))
    {
      GetDIBits (hdc, hbmp,
		 0, 0,
		 NULL,
		 pbmi,
		 0);
    }

#if CYGDEBUG
  ErrorF ("winInitVisuals () - 2nd comp %08x rm %08x gm %08x bm %08x\n",
	  pbmi->bmiHeader.biCompression,
	  pbmih->bV4RedMask,
	  pbmih->bV4GreenMask,
	  pbmih->bV4BlueMask);
#endif

  /* Set default masks if masks could not be detected */
  switch (pScreenInfo->dwDepth)
    {
    case 32:
    case 24:
      if (pbmih->bV4RedMask != 0x00FF0000)
	{
	  pbmih->bV4RedMask = 0x00FF0000;
	}
      if (pbmih->bV4GreenMask != 0x0000FF00)
	{
	  pbmih->bV4GreenMask = 0x0000FF00;
	}
      if (pbmih->bV4BlueMask != 0x000000FF)
	{
	  pbmih->bV4BlueMask = 0x000000FF;
	}
      break;

    case 16:
      if (pbmih->bV4RedMask != 0x0000F800 && pbmih->bV4RedMask != 0x00007C00)
	{
	  pbmih->bV4RedMask = 0x0000F800;
	}
      if (pbmih->bV4GreenMask != 0x00007E0
	  && pbmih->bV4GreenMask != 0x000003E0)
	{
	  pbmih->bV4GreenMask = 0x000007E0;
	}
      if (pbmih->bV4BlueMask != 0x0000001F)
	{	
	  pbmih->bV4BlueMask = 0x0000001F;
	}
      break;
    }

#if CYGDEBUG
  ErrorF ("winInitVisuals () - 3rd comp %08x rm %08x gm %08x bm %08x\n",
	  pbmi->bmiHeader.biCompression,
	  pbmih->bV4RedMask,
	  pbmih->bV4GreenMask,
	  pbmih->bV4BlueMask);
#endif

  /* Copy the bitmasks into the screen privates, for later use */
  pScreenPriv->dwRedMask = pbmih->bV4RedMask;
  pScreenPriv->dwGreenMask = pbmih->bV4GreenMask;
  pScreenPriv->dwBlueMask = pbmih->bV4BlueMask;

  /* Release the DC and the bitmap that were used for querying */
  ReleaseDC (NULL, hdc);
  DeleteObject (hbmp);

  /* Set the significant bits per red, green, and blue */
  switch (pScreenInfo->dwDepth)
    {
    case 32:
    case 24:
      pScreenPriv->dwBitsPerRGB = 8;
      break;
      
    case 16:
      if (pScreenPriv->dwRedMask == 0xF800)
	{
	  pScreenPriv->dwBitsPerRGB = 6;
	}
      else
	{
	  pScreenPriv->dwBitsPerRGB = 5;
	}
      break;
      
    case 15:
      pScreenPriv->dwBitsPerRGB = 5;
      break;
      
    case 8:
      pScreenPriv->dwBitsPerRGB = 8;
      pScreenPriv->dwRedMask = 0;
      pScreenPriv->dwGreenMask = 0;
      pScreenPriv->dwBlueMask = 0;
      break;

    default:
      pScreenPriv->dwBitsPerRGB = 0;
      break;
    }

  /* Tell the user how many bits per RGB we are using */
  ErrorF ("winInitVisuals () - Using dwBitsPerRGB: %d\n",
	  pScreenPriv->dwBitsPerRGB);

  /* Create a single visual according to the Windows screen depth */
  switch (pScreenInfo->dwDepth)
    {
    case 32:
    case 24:
    case 16:
    case 15:
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     TrueColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     TrueColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisuals () - miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;

    case 8:
      ErrorF ("winInitVisuals () - Calling miSetVisualTypesAndMasks\n");
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     PseudoColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     PseudoColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisuals () - miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
#if CYGDEBUG
      ErrorF ("winInitVisuals () - Returned from miSetVisualTypesAndMasks\n");
#endif
      break;

    default:
      break;
    }

  /* Free memory */
  xfree (pbmi);

#if CYGDEBUG
  ErrorF ("winInitVisuals () - Returning\n");
#endif

  return TRUE;
}
