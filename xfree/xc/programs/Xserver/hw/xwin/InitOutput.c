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
/* $XFree86: xc/programs/Xserver/hw/xwin/InitOutput.c,v 1.8 2001/05/08 08:14:09 alanh Exp $ */

#include "win.h"

int		g_nNumScreens;
winScreenInfo	g_winScreens[MAXSCREENS];
int		g_nLastScreen = -1;
ColormapPtr	g_cmInstalledMaps[MAXSCREENS];
int		g_fdMessageQueue = WIN_FD_INVALID;
int		g_winScreenPrivateIndex = -1;
unsigned long	g_winGeneration = 0;

static PixmapFormatRec g_PixmapFormats[] = {
        { 1,    1,      BITMAP_SCANLINE_PAD },
        { 4,    8,      BITMAP_SCANLINE_PAD },
        { 8,    8,      BITMAP_SCANLINE_PAD },
        { 15,   16,     BITMAP_SCANLINE_PAD },
        { 16,   16,     BITMAP_SCANLINE_PAD },
        { 24,   24,     BITMAP_SCANLINE_PAD },
	{ 32,	32,	BITMAP_SCANLINE_PAD }
};
const int NUMFORMATS = sizeof (g_PixmapFormats) / sizeof (g_PixmapFormats[0]);

void
winInitializeDefaultScreens (void)
{
  int                   i;

  for (i = 0; i < MAXSCREENS; i++)
    {
      g_winScreens[i].dwScreen = i;
      g_winScreens[i].dwWidth  = WIN_DEFAULT_WIDTH;
      g_winScreens[i].dwHeight = WIN_DEFAULT_HEIGHT;
      g_winScreens[i].dwDepth  = WIN_DEFAULT_DEPTH;
      g_winScreens[i].pixelBlack = WIN_DEFAULT_BLACKPIXEL;
      g_winScreens[i].pixelWhite = WIN_DEFAULT_WHITEPIXEL;
      g_winScreens[i].dwLineBias = WIN_DEFAULT_LINEBIAS;
      g_winScreens[i].pfb = NULL;
      g_winScreens[i].fFullScreen = FALSE;
      g_winScreens[i].iE3BTimeout = WIN_E3B_OFF;
    }
  g_nNumScreens = 1;
}

DWORD
winBitsPerPixel (DWORD dwDepth)
{
  if (dwDepth == 1) return 1;
  else if (dwDepth <= 8) return 8;
  else if (dwDepth <= 16) return 16;
  else return 32;
}

/* See Porting Layer Definition - p. 57 */
void
ddxGiveUp()
{
#if CYGDEBUG
  ErrorF ("ddxGiveUp ()\n");
#endif

  /* Close our handle to our message queue */
  if (g_fdMessageQueue != WIN_FD_INVALID)
    {
      /* Close /dev/windows */
      close (g_fdMessageQueue);

      /* Set the file handle to invalid */
      g_fdMessageQueue = WIN_FD_INVALID;
    }

  /* Tell Windows that we want to end the app */
  PostQuitMessage (0);
}

/* See Porting Layer Definition - p. 57 */
void
AbortDDX (void)
{
#if CYGDEBUG
  ErrorF ("AbortDDX ()\n");
#endif
  ddxGiveUp ();
}

void
OsVendorInit (void)
{
#if CYGDEBUG
  ErrorF ("OsVendorInit ()\n");
#endif
}

/* See Porting Layer Definition - p. 57 */
void
ddxUseMsg (void)
{
  ErrorF ("-screen n WxHxD\n"\
	  "\tSet screen n's width, height, and bit depth\n");
  ErrorF ("-linebias n\n"\
	  "\tAdjust thin line pixelization\n");
  ErrorF ("-blackpixel n\n"\
	  "\tPixel value for black\n");
  ErrorF ("-whitepixel n\n"\
	  "\tPixel value for white\n");
  ErrorF ("-engine n\n"\
	  "\tOverride the server's detected engine type:\n"\
	  "\t\tGDI blitter\t\t1\n"\
	  "\t\tDirectDraw blitter\t2\n"\
	  "\t\tDirectDraw4 blitter\t4\n");
  ErrorF ("-fullscreen\n"
	  "\tRun the specified server engine in fullscreen mode\n");
  ErrorF ("-emulate3buttons [n]\n"
	  "\tEmulate 3 button mouse with timeout of n milliseconds\n");
}

/* See Porting Layer Definition - p. 57 */
int
ddxProcessArgument (int argc, char *argv[], int i)
{
  static Bool		fFirstTime = TRUE;

  /* Run some initialization procedures the first time through */
  if (fFirstTime)
    {
      winInitializeDefaultScreens ();
      fFirstTime = FALSE;
    }
  
  /*
   * Look for the '-screen n WxHxD' arugment
   */
  if (strcmp (argv[i], "-screen") == 0)
    {
      int		nScreenNum;

      /* Display the usage message if the argument is malformed */
      if (i + 2 >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      nScreenNum = atoi (argv[i+1]);

      /* Validate the specified screen number */
      if (nScreenNum < 0 || nScreenNum >= MAXSCREENS)
        {
          ErrorF ("Invalid screen number %d\n", nScreenNum);
          UseMsg ();
	  return 0;
        }
      
      /* Grab the height, width, and depth parameters */
      if (3 != sscanf (argv[i+2], "%dx%dx%d",
		       (int*)&g_winScreens[nScreenNum].dwWidth,
		       (int*)&g_winScreens[nScreenNum].dwHeight,
		       (int*)&g_winScreens[nScreenNum].dwDepth))
        {
	  /* I see no height, width, and depth here */
          ErrorF ("Invalid screen configuration %s\n", argv[i+2]);
          UseMsg ();
	  return 0;
        }

      if (nScreenNum >= g_nNumScreens)
        g_nNumScreens = nScreenNum + 1;
      g_nLastScreen = nScreenNum;
      return 3;
    }

  /*
   * Look for the '-blackpixel n' argument
   */
  if (strcmp (argv[i], "-blackpixel") == 0)     /* -blackpixel n */
    {
      Pixel		pix;

      /* Display the usage message if the argument is malformed */
      if (++i >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      pix = atoi (argv[i]);

      /* Is this parameter attached to a screen or global? */
      if (-1 == g_nLastScreen)
        {
          int			j;

	  /* Parameter is for all screens */
          for (j = 0; j < MAXSCREENS; j++)
            {
              g_winScreens[j].pixelBlack = pix;
            }
        }
      else
        {
	  /* Parameter is for a single screen */
          g_winScreens[g_nLastScreen].pixelBlack = pix;
        }
      return 2;
    }

  /*
   * Look for the '-whitepixel n' argument
   */
  if (strcmp (argv[i], "-whitepixel") == 0)     /* -whitepixel n */
    {
      Pixel		pix;

      /* Display the usage message if the argument is malformed */
      if (++i >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      pix = atoi (argv[i]);

      /* Is this parameter attached to a screen or global? */
      if (-1 == g_nLastScreen)
        {
          int			j;

	  /* Parameter is for all screens */
          for (j = 0; j < MAXSCREENS; j++)
            {
              g_winScreens[j].pixelWhite = pix;
            }
        }
      else
        {
	  /* Parameter is for a single screen */
          g_winScreens[g_nLastScreen].pixelWhite = pix;
        }
      return 2;
    }

  /*
   * Look for the '-linebias n' argument
   */
  if (strcmp (argv[i], "-linebias") == 0)
    {
      unsigned int	uiLinebias;

      /* Display the usage message if the argument is malformed */
      if (++i >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      uiLinebias = atoi (argv[i]);

      /* Is this parameter attached to a screen or global? */
      if (-1 == g_nLastScreen)
        {
          int		j;

	  /* Parameter is for all screens */
          for (j = 0; j < MAXSCREENS; j++)
            {
              g_winScreens[j].dwLineBias = uiLinebias;
            }
        }
      else
        {
	  /* Parameter is for a single screen */
          g_winScreens[g_nLastScreen].dwLineBias = uiLinebias;
        }
      return 2;
    }

  /*
   * Look for the '-engine n' argument
   */
  if (strcmp (argv[i], "-engine") == 0)
    {
      DWORD		dwEngine = 0;
      CARD8		c8OnBits = 0;
      
      /* Display the usage message if the argument is malformed */
      if (++i >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      /* Grab the argument */
      dwEngine = atoi (argv[i]);

      /* Count the one bits in the engine argument */
      c8OnBits = winCountBits (dwEngine);

      /* Argument should only have a single bit on */
      if (c8OnBits != 1)
	{
	  UseMsg ();
	  return 0;
	}

      /* Is this parameter attached to a screen or global? */
      if (-1 == g_nLastScreen)
	{
	  int		j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_winScreens[j].dwEnginePreferred = dwEngine;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_winScreens[g_nLastScreen].dwEnginePreferred = dwEngine;
	}
      
      /* Indicate that we have processed the argument */
      return 2;
    }

  /*
   * Look for the '-fullscreen' argument
   */
  if (strcmp(argv[i], "-fullscreen") == 0)
    {
      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_nLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_winScreens[j].fFullScreen = TRUE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_winScreens[g_nLastScreen].fFullScreen = TRUE;
	}

      /* Indicate that we have processed this argument */
      return 1;
    }

  /*
   * Look for the '-emulate3buttons' argument
   */
  if (strcmp(argv[i], "-emulate3buttons") == 0)
    {
      int	iArgsProcessed = 1;
      int	iE3BTimeout = WIN_DEFAULT_E3B_TIME;

      /* Grab the optional timeout value */
      if (i + 1 < argc
	  && 1 == sscanf (argv[i + 1], "%d",
			  &iE3BTimeout))
        {
	  /* Indicate that we have processed the next argument */
	  iArgsProcessed++;
        }
      else
	{
	  /*
	   * sscanf () won't modify iE3BTimeout if it doesn't find
	   * the specified format; however, I want to be explicit
	   * about setting the default timeout in such cases to
	   * prevent some programs (me) from getting confused.
	   */
	  iE3BTimeout = WIN_DEFAULT_E3B_TIME;
	}

      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_nLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_winScreens[j].iE3BTimeout = iE3BTimeout;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_winScreens[g_nLastScreen].iE3BTimeout = TRUE;
	}

      /* Indicate that we have processed this argument */
      return iArgsProcessed;
    }

  return 0;
}

#ifdef DDXTIME /* from ServerOSDefines */
CARD32
GetTimeInMillis (void)
{
  return GetTickCount ();
}
#endif /* DDXTIME */

/* See Porting Layer Definition - p. 20 */
/* We use ddxProcessArgument, so we don't need to touch argc and argv */
void
InitOutput (ScreenInfo *screenInfo, int argc, char *argv[])
{
  int		i;

  /* Setup global screen info parameters */
  screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
  screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
  screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
  screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
  screenInfo->numPixmapFormats = NUMFORMATS;

  /* Describe how we want common pixmap formats padded */
  for (i = 0; i < NUMFORMATS; i++)
    {
      screenInfo->formats[i] = g_PixmapFormats[i];
    }

  /* Initialize each screen */
  for (i = 0; i < g_nNumScreens; i++)
    {
      if (-1 == AddScreen (winScreenInit, argc, argv))
	{
	  FatalError ("Couldn't add screen %d", i);
	}
    }
}
