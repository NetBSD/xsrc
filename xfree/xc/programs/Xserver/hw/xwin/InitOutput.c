/* $TOG: InitOutput.c /main/20 1998/02/10 13:23:56 kaleb $ */
/*

Copyright 1993, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/hw/xwin/InitOutput.c,v 1.2 2000/11/28 16:10:40 dawes Exp $ */

#if defined(WIN32) && !defined(__CYGWIN__)
#include <X11/Xwinsock.h>
#endif
#include <stdio.h>
#include "X11/X.h"
#define NEED_EVENTS
#include "X11/Xproto.h"
#include "X11/Xos.h"
#include "scrnintstr.h"
#include "servermd.h"
#define PSZ 8
#include "cfb.h"
#include "mibstore.h"
#include "colormapst.h"
#include "gcstruct.h"
#include "input.h"
#include "mipointer.h"
#include "picturestr.h"
#include <sys/types.h>
#ifdef HAS_MMAP
#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#endif /* HAS_MMAP */
#include <sys/stat.h>
#include <errno.h>
#ifndef WIN32
#include <sys/param.h>
#endif
#include <X11/XWDFile.h>
#if defined(HAS_SHM) && !defined(__CYGWIN__)
#include <sys/cygipc.h>
#include <sys/shm.h>
#endif /* HAS_SHM */
#include "dix.h"
#include "miline.h"

extern char     *display;
extern          void winfbBlockHandler ();
extern          int winfbWakeupHandler ();

#define WIN_DEFAULT_WIDTH  1280
#define WIN_DEFAULT_HEIGHT 1024
#define WIN_DEFAULT_DEPTH  16
#define WIN_DEFAULT_WHITEPIXEL 255
#define WIN_DEFAULT_BLACKPIXEL 0
#define WIN_DEFAULT_LINEBIAS 0
#define XWD_WINDOW_NAME_LEN 60

typedef struct
{
  int scrnum;
  int width;
  int paddedWidth;
  int height;
  int depth;
  int bitsPerPixel;
  char *pfbMemory;
  XWDColor *pXWDCmap;
  XWDFileHeader *pXWDHeader;
  Pixel blackPixel;
  Pixel whitePixel;
  ScreenPtr pScreen ;
  unsigned int lineBias;

} winScreenInfo, *winScreenInfoPtr;

typedef enum { NORMAL_MEMORY_FB, SHARED_MEMORY_FB, MMAPPED_FILE_FB } fbMemType;

static int              winNumScreens;
static winScreenInfo    winScreens[MAXSCREENS];
static Bool             winPixmapDepths[33];
static char             *pfbdir = NULL;
static fbMemType        fbmemtype = NORMAL_MEMORY_FB;
static char             needswap = 0;
static int              lastScreen = -1;
static ColormapPtr      InstalledMaps[MAXSCREENS];

#define swapcopy16(_dst, _src) \
    if (needswap) { CARD16 _s = _src; cpswaps(_s, _dst); } \
    else _dst = _src;

#define swapcopy32(_dst, _src) \
    if (needswap) { CARD32 _s = _src; cpswapl(_s, _dst); } \
    else _dst = _src;

static void
winInitializePixmapDepths()
{
  int i;
  winPixmapDepths[1] = TRUE; /* always need bitmaps */
  for (i = 2; i <= 32; i++)
    winPixmapDepths[i] = FALSE;
}

static Bool
winCreateDefColormap (ScreenPtr pScreen)
{
  int                   i;
  Pixel                 wp, bp;
  VisualPtr             pVisual;
  ColormapPtr           cmap;
  unsigned short        zero = 0, ones = 0xFFFF;

  /*
   * these are the MS-Windows desktop colors, adjusted for X's 16-bit
   * color specifications.
   */
  static xColorItem citems[] = {
    {   0,      0,      0,      0, 0, 0 },
    {   1, 0x8000,      0,      0, 0, 0 },
    {   2,      0, 0x8000,      0, 0, 0 },
    {   3, 0x8000, 0x8000,      0, 0, 0 },
    {   4,      0,      0, 0x8000, 0, 0 },
    {   5, 0x8000,      0, 0x8000, 0, 0 },
    {   6,      0, 0x8000, 0x8000, 0, 0 },
    {   7, 0xc000, 0xc000, 0xc000, 0, 0 },
    {   8, 0xc000, 0xdc00, 0xc000, 0, 0 },
    {   9, 0xa600, 0xca00, 0xf000, 0, 0 },
    { 246, 0xff00, 0xfb00, 0xf000, 0, 0 },
    { 247, 0xa000, 0xa000, 0xa400, 0, 0 },
    { 248, 0x8000, 0x8000, 0x8000, 0, 0 },
    { 249, 0xff00,      0,      0, 0, 0 },
    { 250,      0, 0xff00,      0, 0, 0 },
    { 251, 0xff00, 0xff00,      0, 0, 0 },
    { 252,      0,      0, 0xff00, 0, 0 },
    { 253, 0xff00,      0, 0xff00, 0, 0 },
    { 254,      0, 0xff00, 0xff00, 0, 0 },
    { 255, 0xff00, 0xff00, 0xff00, 0, 0 }
  };
#define NUM_DESKTOP_COLORS sizeof citems / sizeof citems[0]

  for (pVisual = pScreen->visuals;
       pVisual->vid != pScreen->rootVisual;
       pVisual++);

  if (CreateColormap (pScreen->defColormap, pScreen, pVisual, &cmap,
                      (pVisual->class & DynamicClass) ? AllocNone : AllocAll,
                      0)
      != Success)
    return FALSE;
  if (pVisual->class == PseudoColor)
    {
      for (i = 0; i < NUM_DESKTOP_COLORS; i++)
        {
          if (AllocColor (cmap,
                          &citems[i].red,
                          &citems[i].green,
                          &citems[i].blue,
                          &citems[i].pixel,
                          0) != Success)
            return FALSE;
        }
      pScreen->whitePixel = 255;
      pScreen->blackPixel = 0;
    }
  else
    {
      if ((AllocColor (cmap, &ones, &ones, &ones, &wp, 0)
           != Success)
          || (AllocColor (cmap, &zero, &zero, &zero, &bp, 0)
              != Success))
        return FALSE;
      pScreen->whitePixel = wp;
      pScreen->blackPixel = bp;
    }
  (*pScreen->InstallColormap)(cmap);
  return TRUE;
}

static void
winInitializeDefaultScreens()
{
  int                   i;

  for (i = 0; i < MAXSCREENS; i++)
    {
      winScreens[i].scrnum = i;
      winScreens[i].width  = WIN_DEFAULT_WIDTH;
      winScreens[i].height = WIN_DEFAULT_HEIGHT;
      winScreens[i].depth  = WIN_DEFAULT_DEPTH;
      winScreens[i].blackPixel = WIN_DEFAULT_BLACKPIXEL;
      winScreens[i].whitePixel = WIN_DEFAULT_WHITEPIXEL;
      winScreens[i].lineBias = WIN_DEFAULT_LINEBIAS;
      winScreens[i].pfbMemory = NULL;
    }
  winNumScreens = 1;
}

static int
winBitsPerPixel (int depth)
{
  if (depth == 1) return 1;
  else if (depth <= 8) return 8;
  else if (depth <= 16) return 16;
  else if (depth <= 24) return 24;
  else return 32;
}

void
ddxGiveUp()
{
  return;
}

void
AbortDDX()
{
  ddxGiveUp();
}

void
OsVendorInit()
{
}

void
ddxUseMsg()
{
  ErrorF("-screen scrn WxHxD     set screen's width, height, bit depth\n");
  ErrorF("-pixdepths list-of-int support given pixmap depths\n");
  ErrorF("-linebias n            adjust thin line pixelization\n");
  ErrorF("-blackpixel n          pixel value for black\n");
  ErrorF("-whitepixel n          pixel value for white\n");
}

int
ddxProcessArgument (int argc, char *argv[], int i)
{
  static Bool firstTime = TRUE;

  if (firstTime)
    {
      winInitializeDefaultScreens();
      winInitializePixmapDepths();
      firstTime = FALSE;
    }

  if (strcmp (argv[i], "-screen") == 0) /* -screen n WxHxD */
    {
      int screenNum;
      if (i + 2 >= argc) UseMsg();
      screenNum = atoi(argv[i+1]);
      if (screenNum < 0 || screenNum >= MAXSCREENS)
        {
          ErrorF("Invalid screen number %d\n", screenNum);
          UseMsg();
        }
      if (3 != sscanf(argv[i+2], "%dx%dx%d",
                      &winScreens[screenNum].width,
                      &winScreens[screenNum].height,
                      &winScreens[screenNum].depth))
        {
          ErrorF("Invalid screen configuration %s\n", argv[i+2]);
          UseMsg();
        }

      if (screenNum >= winNumScreens)
        winNumScreens = screenNum + 1;
      lastScreen = screenNum;
      return 3;
    }

  if (strcmp (argv[i], "-pixdepths") == 0)      /* -pixdepths list-of-depth */
    {
      int depth, ret = 1;

      if (++i >= argc) UseMsg();
      while ((i < argc) && (depth = atoi(argv[i++])) != 0)
        {
          if (depth < 0 || depth > 32)
            {
              ErrorF("Invalid pixmap depth %d\n", depth);
              UseMsg();
            }
          winPixmapDepths[depth] = TRUE;
          ret++;
        }
      return ret;
    }

  if (strcmp (argv[i], "-blackpixel") == 0)     /* -blackpixel n */
    {
      Pixel pix;
      if (++i >= argc) UseMsg();
      pix = atoi(argv[i]);
      if (-1 == lastScreen)
        {
          int i;
          for (i = 0; i < MAXSCREENS; i++)
            {
              winScreens[i].blackPixel = pix;
            }
        }
      else
        {
          winScreens[lastScreen].blackPixel = pix;
        }
      return 2;
    }

  if (strcmp (argv[i], "-whitepixel") == 0)     /* -whitepixel n */
    {
      Pixel pix;
      if (++i >= argc) UseMsg();
      pix = atoi(argv[i]);
      if (-1 == lastScreen)
        {
          int i;
          for (i = 0; i < MAXSCREENS; i++)
            {
              winScreens[i].whitePixel = pix;
            }
        }
      else
        {
          winScreens[lastScreen].whitePixel = pix;
        }
      return 2;
    }

  if (strcmp (argv[i], "-linebias") == 0)       /* -linebias n */
    {
      unsigned int linebias;
      if (++i >= argc) UseMsg();
      linebias = atoi(argv[i]);
      if (-1 == lastScreen)
        {
          int i;
          for (i = 0; i < MAXSCREENS; i++)
            {
              winScreens[i].lineBias = linebias;
            }
        }
      else
        {
          winScreens[lastScreen].lineBias = linebias;
        }
      return 2;
    }

  if (strcmp (argv[i], "-probe") == 0)  /* -linebias n */
    {
      ListModes ();
      return 1;
    }

  return 0;
}

#ifdef DDXTIME /* from ServerOSDefines */
CARD32
GetTimeInMillis()
{
  struct timeval  tp;

  X_GETTIMEOFDAY(&tp);
  return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
#endif

static Bool
winMultiDepthCreateGC (GCPtr pGC)
{
  switch (winBitsPerPixel(pGC->depth))
    {
    case 1:  return mfbCreateGC (pGC);
    case 8:  return cfbCreateGC (pGC);
    case 16: return cfb16CreateGC (pGC);
    case 24: return cfb24CreateGC (pGC);
    case 32: return cfb32CreateGC (pGC);
    default: return FALSE;
    }
}

static void
winMultiDepthGetSpans (DrawablePtr pDrawable, int wMax,
                       register DDXPointPtr ppt,
                       int *pwidth, int nspans, char *pdstStart)
{
  switch (pDrawable->bitsPerPixel)
    {
    case 1:
      mfbGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
      break;
    case 8:
      cfbGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
      break;
    case 16:
      cfb16GetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
      break;
    case 24:
      cfb24GetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
      break;
    case 32:
      cfb32GetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
      break;
    }
  return;
}

static void
winMultiDepthGetImage (DrawablePtr pDrawable, int sx, int sy,
                       int w, int h, unsigned int format,
                       unsigned long planeMask,
                       char *pdstLine)
{
  switch (pDrawable->bitsPerPixel)
    {
    case 1:
      mfbGetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
      break;
    case 8:
      cfbGetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
      break;
    case 16:
      cfb16GetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
      break;
    case 24:
      cfb24GetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
      break;
    case 32:
      cfb32GetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
      break;
    }
}

static int
winListInstalledColormaps (ScreenPtr pScreen, Colormap *pmaps)
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}

static void
winInstallColormap (ColormapPtr pmap)
{
  int                   index = pmap->pScreen->myNum;
  ColormapPtr           oldpmap = InstalledMaps[index];
  int                   entries;
  VisualPtr             pVisual;
  Pixel                 *ppix;
  xrgb                  *prgb;
  xColorItem            *defs;
  int                   i;

  if (pmap != oldpmap)
    {
      if(oldpmap != (ColormapPtr)None)
        {
          WalkTree (pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
        }

      /* Install pmap */
      InstalledMaps[index] = pmap;
      WalkTree (pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

      entries = pmap->pVisual->ColormapEntries;
      pVisual = pmap->pVisual;

      ppix = (Pixel *)ALLOCATE_LOCAL(entries * sizeof(Pixel));
      prgb = (xrgb *)ALLOCATE_LOCAL(entries * sizeof(xrgb));
      defs = (xColorItem *)ALLOCATE_LOCAL(entries * sizeof(xColorItem));

      for (i = 0; i < entries; i++)
        {
          ppix[i] = i;
        }

      /* XXX truecolor */
      QueryColors(pmap, entries, ppix, prgb);

      /* convert xrgbs to xColorItems */
      for (i = 0; i < entries; i++)
        {
          defs[i].pixel = ppix[i] & 0xff; /* change pixel to index */
          defs[i].red = prgb[i].red;
          defs[i].green = prgb[i].green;
          defs[i].blue = prgb[i].blue;
          defs[i].flags =  DoRed|DoGreen|DoBlue;
        }
      (*pmap->pScreen->StoreColors)(pmap, entries, defs);

      DEALLOCATE_LOCAL(ppix);
      DEALLOCATE_LOCAL(prgb);
      DEALLOCATE_LOCAL(defs);
    }
}

static void
winUninstallColormap (ColormapPtr pmap)
{
  ColormapPtr           curpmap = InstalledMaps[pmap->pScreen->myNum];

  if (pmap == curpmap)
    {
      if (pmap->mid != pmap->pScreen->defColormap)
        {
          curpmap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
                                                 RT_COLORMAP);
          (*pmap->pScreen->InstallColormap)(curpmap);
        }
    }
}

static void
winStoreColors (ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
  int                   i;

  if (pmap != InstalledMaps[pmap->pScreen->myNum]) return;

  if ((pmap->pVisual->class | DynamicClass) == DirectColor)
    return;

  // TrueColor or Pseudo Color
  if (pmap->pVisual->class == PseudoColor)
    {
      for (i = 0; i < ndef; i++, pdefs++)
        {
          DXStoreColors (pdefs->pixel,
                         pdefs->red,
                         pdefs->green,
                         pdefs->blue);
        }
    }
}

static Bool
winSaveScreen (ScreenPtr pScreen, int on)
{
    return TRUE;
}

static char *
winAllocateFramebufferMemory (winScreenInfoPtr pwin)
{
  pwin->pfbMemory = winDXAllocateFramebufferMemory (&pwin->width,
                                                    &pwin->height,
                                                    &pwin->depth,
                                                    &pwin->paddedWidth);
  return pwin->pfbMemory;
}

static Bool
winCursorOffScreen (ScreenPtr *ppScreen, int *x, int *y)
{
  return FALSE;
}

static void
winCrossScreen (ScreenPtr pScreen, Bool entering)
{
}

static miPointerScreenFuncRec winPointerCursorFuncs =
{
  winCursorOffScreen,
  winCrossScreen,
  miPointerWarpCursor
};

Bool miCreateScreenResources (ScreenPtr pScreen) ;

static Bool
winScreenInit (int index, ScreenPtr pScreen, int argc, char *argv[])
{
  winScreenInfoPtr      pwin = &winScreens[index];
  PictFormatPtr         formats = NULL;
  int                   nformats = 0;
  int                   dpix = 100, dpiy = 100;
  int                   ret = FALSE;
  char                  *pbits;
  BOOL                  fResult = FALSE;
  unsigned long         stack_ptr;

  /* Initial display parameters */
  pwin->paddedWidth = PixmapBytePad(pwin->width, pwin->depth);
  pwin->bitsPerPixel = winBitsPerPixel(pwin->depth);

  pbits = winAllocateFramebufferMemory (pwin);

  if (!pbits) return FALSE;

  fprintf (stderr, "Obtained Width: %d, Height: %d, Depth: %d\n",
           pwin->width, pwin->height, pwin->bitsPerPixel);

  switch (pwin->bitsPerPixel)
    {
    case 1:
      ret = mfbScreenInit (pScreen, pbits, pwin->width, pwin->height,
                           dpix, dpiy, pwin->paddedWidth * 8);
      break;
    case 8:
      fprintf( stderr, "Calling cfbScreenInit (%08x, %08x, %d, %d, %d, %d, %d)\n",
               pScreen, pbits, pwin->width, pwin->height,
               dpix, dpiy, pwin->paddedWidth);

      ret = cfbScreenInit (pScreen, pbits, pwin->width, pwin->height,
                           dpix, dpiy, pwin->paddedWidth);
      break;
    case 16:
      fprintf( stderr, "Calling cfb16ScreenInit (%08x, %08x, %d, %d, %d, %d, %d)\n",
               pScreen, pbits, pwin->width, pwin->height,
               dpix, dpiy, pwin->paddedWidth);
      ret = cfb16ScreenInit (pScreen, pbits, pwin->width, pwin->height,
                             dpix, dpiy, pwin->paddedWidth);
      break;
    case 24:
      fprintf( stderr, "Calling cfb24ScreenInit (%08x, %08x, %d, %d, %d, %d, %d)\n",
               pScreen, pbits, pwin->width, pwin->height,
               dpix, dpiy, pwin->paddedWidth);
      ret = cfb24ScreenInit (pScreen, pbits, pwin->width, pwin->height,
                             dpix, dpiy, pwin->paddedWidth);
      break;
    case 32:
      fprintf( stderr, "Calling cfb32ScreenInit (%08x, %08x, %d, %d, %d, %d, %d)\n",
               pScreen, pbits, pwin->width, pwin->height,
               dpix, dpiy, pwin->paddedWidth);

      ret = cfb32ScreenInit (pScreen, pbits, pwin->width, pwin->height,
                             dpix, dpiy, pwin->paddedWidth);
      break;
    default:
      return FALSE;
    }

  if (!ret) return FALSE;

  pScreen->CreateGC = winMultiDepthCreateGC;
  pScreen->GetImage = winMultiDepthGetImage;
  pScreen->GetSpans = winMultiDepthGetSpans;

  pScreen->InstallColormap = winInstallColormap;
  pScreen->UninstallColormap = winUninstallColormap;
  pScreen->ListInstalledColormaps = winListInstalledColormaps;

  pScreen->SaveScreen = winSaveScreen;
  pScreen->StoreColors = winStoreColors;

  miPictureInit(pScreen, formats, nformats);
  miDCInitialize (pScreen, &winPointerCursorFuncs);

  pScreen->blackPixel = pwin->blackPixel;
  pScreen->whitePixel = pwin->whitePixel;

  if (pwin->bitsPerPixel == 1)
    {
      ret = mfbCreateDefColormap (pScreen);
    }
  else
    {
      ret = winCreateDefColormap (pScreen);
    }

  miSetZeroLineBias (pScreen, pwin->lineBias);

  if (ret)
    {
      RegisterBlockAndWakeupHandlers (winfbBlockHandler,
				      winfbWakeupHandler,
				      NULL);
    }
  pwin->pScreen = pScreen ;
  ErrorF ("Successful addition of Screen %p %p\n",
	  pScreen->devPrivate,
	  pScreen);
  return ret;

}

void
InitOutput (ScreenInfo *screenInfo, int argc, char *argv[])
{
  int		i;
  int		iNumFormats = 0;
  int		*piDepth = 0, *piWidth = 0, *piHeight = 0;
  FILE		*pf = stderr;

  /* Initialize pixmap formats */

  /* cfbLoad() ;*/

  /* Adjust bit depth for all screens */
  for (i = 0; i < winNumScreens; i++)
    {
      piWidth = &winScreens[i].width;
      piHeight = &winScreens[i].height;
      piDepth = &winScreens[i].depth;

      fprintf (stderr, "Desired Width: %d, Height: %d, Depth: %d\n",
	       *piWidth, *piHeight, *piDepth);

      /* Adjust bit depth and display size for hardware requirements */
      AdjustVideoMode (piWidth, piHeight, piDepth, TRUE);

      fprintf (stderr, "Adjusted Width: %d, Height: %d, Depth: %d\n",
	       *piWidth, *piHeight, *piDepth);
    }

  /* must have a pixmap depth to match every screen depth */
  for (i = 0; i < winNumScreens; i++)
    {
      winPixmapDepths[winScreens[i].depth] = TRUE;
    }

  for (i = 1; i <= 32; i++)
    {
      if (winPixmapDepths[i])
	{
	  if (iNumFormats >= MAXFORMATS)
	    FatalError ("MAXFORMATS is too small for this server\n");
	  screenInfo->formats[iNumFormats].depth = i;
	  screenInfo->formats[iNumFormats].bitsPerPixel = winBitsPerPixel(i);
	  screenInfo->formats[iNumFormats].scanlinePad = BITMAP_SCANLINE_PAD;
	  iNumFormats++;
	}
    }

  screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
  screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
  screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
  screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
  screenInfo->numPixmapFormats = iNumFormats;

  /* initialize screens */
  for (i = 0; i < winNumScreens; i++)
    {
      if (-1 == AddScreen (winScreenInit, argc, argv))
	{
	  FatalError ("Couldn't add screen %d", i);
	}
    }
}

void
SwitchFramebuffer (pointer pbits)
{
  PixmapPtr		pmap;
  ScreenPtr		s = winScreens[0].pScreen;

  pmap = ((PixmapPtr) (s)->devPrivate);
  /*
    if (winScreens[0].depth == 8)
    pmap = ((PixmapPtr) (s)->devPrivate);
    else
    pmap = ((PixmapPtr) (s)->devPrivates[0].ptr);
  */
  /*ErrorF ("Switch: %p, %p\n", winScreens[0].pScreen, pmap);*/
  pmap->devPrivate.ptr = pbits;
}
