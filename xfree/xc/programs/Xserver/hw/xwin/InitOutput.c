/* $TOG: InitOutput.c /main/20 1998/02/10 13:23:56 kaleb $ */
/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

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
/* $XFree86: xc/programs/Xserver/hw/xwin/InitOutput.c,v 1.27 2001/12/14 19:59:53 dawes Exp $ */

#include "win.h"

int		g_iNumScreens = 0;
winScreenInfo	g_ScreenInfo[MAXSCREENS];
int		g_iLastScreen = -1;
int		g_fdMessageQueue = WIN_FD_INVALID;
int		g_iScreenPrivateIndex = -1;
int		g_iCmapPrivateIndex = -1;
int		g_iGCPrivateIndex = -1;
int		g_iPixmapPrivateIndex = -1;
unsigned long	g_ulServerGeneration = 0;
Bool		g_fInitializedDefaultScreens = FALSE;
FILE		*g_pfLog = NULL;

extern void OsVendorVErrorF (const char *pszFormat, va_list va_args);

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
  DWORD			dwWidth, dwHeight;

  /* Bail out early if default screens have already been initialized */
  if (g_fInitializedDefaultScreens)
    return;

  /* Zero the memory used for storing the screen info */
  ZeroMemory (g_ScreenInfo, MAXSCREENS * sizeof (winScreenInfo));

  /* Get default width and height */
  dwWidth = GetSystemMetrics (SM_CXSCREEN);
  dwHeight = GetSystemMetrics (SM_CYSCREEN);

  ErrorF ("winInitializeDefaultScreens () - w %d h %d\n", dwWidth, dwHeight);

  /* Set a default DPI, if no parameter was passed */
  if (monitorResolution == 0)
    monitorResolution = WIN_DEFAULT_DPI;

  for (i = 0; i < MAXSCREENS; ++i)
    {
      g_ScreenInfo[i].dwScreen = i;
      g_ScreenInfo[i].dwWidth  = dwWidth;
      g_ScreenInfo[i].dwHeight = dwHeight;
      g_ScreenInfo[i].dwDepth  = WIN_DEFAULT_DEPTH;
      g_ScreenInfo[i].dwRefreshRate = WIN_DEFAULT_REFRESH;
      g_ScreenInfo[i].pfb = NULL;
      g_ScreenInfo[i].fFullScreen = FALSE;
      g_ScreenInfo[i].iE3BTimeout = WIN_E3B_OFF;
      g_ScreenInfo[i].dwWidth_mm = (dwWidth / WIN_DEFAULT_DPI)
	* 25.4;
      g_ScreenInfo[i].dwHeight_mm = (dwHeight / WIN_DEFAULT_DPI)
	* 25.4;
      g_ScreenInfo[i].fUseWinKillKey = WIN_DEFAULT_WIN_KILL;
      g_ScreenInfo[i].fUseUnixKillKey = WIN_DEFAULT_UNIX_KILL;
      g_ScreenInfo[i].fIgnoreInput = FALSE;
    }

  /* Signal that the default screens have been initialized */
  g_fInitializedDefaultScreens = TRUE;
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

  /* Close the log file handle */
  if (g_pfLog != NULL)
    {
      /* Close log file */
      fclose (g_pfLog);
      
      /* Set the file handle to invalid */
      g_pfLog = NULL;
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
#ifdef DDXOSVERRORF
  if (!OsVendorVErrorFProc)
    OsVendorVErrorFProc = OsVendorVErrorF;

  /* Open log file if not yet open */
  if (g_pfLog == NULL)
    g_pfLog = fopen (WIN_LOG_FNAME, "w");
#endif

  /* Add a default screen if no screens were specified */
  if (g_iNumScreens == 0)
    {
      /* 
       * We need to initialize default screens if no arguments
       * were processed.  Otherwise, the default screens would
       * already have been initialized by ddxProcessArgument ().
       */
      winInitializeDefaultScreens ();

      /*
       * Add a screen 0 using the defaults set by 
       * winInitializeDefaultScreens () and any additional parameters
       * processed by ddxProcessArgument ().
       */
      g_iNumScreens = 1;
      g_iLastScreen = 0;
    }
}


/* See Porting Layer Definition - p. 57 */
void
ddxUseMsg (void)
{
  ErrorF ("-depth bits_per_pixel\n"
	  "\tSpecify an optional bitdepth to use in fullscreen mode\n"
	  "\twith a DirectDraw engine.\n");

  ErrorF ("-emulate3buttons [timeout]\n"
	  "\tEmulate 3 button mouse with an optional timeout in "
	  "milliseconds\n");

  ErrorF ("-engine engine_type_id\n"
	  "\tOverride the server's automatically selected engine type:\n"
	  "\t\t1 - Shadow GDI\n"
	  "\t\t2 - Shadow DirectDraw\n"
	  "\t\t4 - Shadow DirectDraw4\n"
	  "\t\t16 - Native GDI - experimental\n");

  ErrorF ("-fullscreen\n"
	  "\tRun the server in fullscreen mode\n");
  
  ErrorF ("-refresh rate_in_Hz\n"
	  "\tSpecify an optional refresh rate to use in fullscreen mode\n"
	  "\twith a DirectDraw engine.\n");

  ErrorF ("-screen scr_num width height\n"
	  "\tSet screen scr_num's width and height\n");

  ErrorF ("-[no]unixkill\n"
          "\tCtrl+Alt+Backspace exits the X Server\n");

  ErrorF ("-[no]winkill\n"
          "\tAlt+F4 exits the X Server\n");
}


/* See Porting Layer Definition - p. 57 */
/*
 * INPUT
 * argv: pointer to an array of null-terminated strings, one for
 *   each token in the X Server command line; the first token
 *   is 'XWin.exe', or similar.
 * argc: a count of the number of tokens stored in argv.
 * i: a zero-based index into argv indicating the current token being
 *   processed.
 *
 * OUTPUT
 * return: return the number of tokens processed correctly.
 *
 * NOTE
 * When looking for n tokens, check that i + n is less than argc.  Or,
 *   you may check if i is greater than or equal to argc, in which case
 *   you should display the UseMsg () and return 0.
 */

int
ddxProcessArgument (int argc, char *argv[], int i)
{
  static Bool		beenHere = FALSE;

  /* Initialize once */
  if (!beenHere)
    {
#ifdef DDXOSVERRORF
      /*
       * This initialises our hook into VErrorF () for catching log messages
       * that are generated before OsInit () is called.
       */
      OsVendorVErrorFProc = OsVendorVErrorF;

      /* Open log file if not yet open */
      if (g_pfLog == NULL)
	g_pfLog = fopen (WIN_LOG_FNAME, "w");
#endif

      beenHere = TRUE;

      /* Detach from any console we are connected to */
      FreeConsole ();

      /*
       * Initialize default screen settings.  We have to do this before
       * OsVendorInit () gets called, otherwise we will overwrite
       * settings changed by parameters such as -fullscreen, etc.
       */
      ErrorF ("ddxProcessArgument () - Initializing default screens\n");
      winInitializeDefaultScreens ();
  }

#if CYGDEBUG
  ErrorF ("ddxProcessArgument ()\n");
#endif
  
  /*
   * Look for the '-screen scr_num width height' argument
   */
  if (strcmp (argv[i], "-screen") == 0)
    {
      int		iArgsProcessed = 1;
      int		nScreenNum;

      ErrorF ("ddxProcessArgument () - screen - argc: %d i: %d\n",
	      argc, i);

      /* Display the usage message if the argument is malformed */
      if (i + 2 >= argc)
	{
	  return 0;
	}
      
      /* Grab screen number */
      nScreenNum = atoi (argv[i + 1]);

      /* Validate the specified screen number */
      if (nScreenNum < 0 || nScreenNum >= MAXSCREENS)
        {
          ErrorF ("ddxProcessArgument () - Invalid screen number %d\n",
		  nScreenNum);
          UseMsg ();
	  return 0;
        }

      /* Look for 'WxD' or 'W D' */
      if (2 == sscanf (argv[i + 2], "%dx%d",
		       (int *) &g_ScreenInfo[nScreenNum].dwWidth,
		       (int *) &g_ScreenInfo[nScreenNum].dwHeight))
	{
	  iArgsProcessed = 3;
	}
      else if (i + 3 < argc
	       && 1 == sscanf (argv[i + 2], "%d",
			       (int *) &g_ScreenInfo[nScreenNum].dwWidth)
	       && 1 == sscanf (argv[i + 3], "%d",
			       (int *) &g_ScreenInfo[nScreenNum].dwHeight))
	{
	  iArgsProcessed = 4;
	}
      else
	{
	  /* I see no height and width here */
          ErrorF ("ddxProcessArgument () - Invalid screen width and "
		  "height: %s\n",
		  argv[i + 2]);
	  return 0;
	}


      /* Calculate the screen width and height in millimeters */
      g_ScreenInfo[nScreenNum].dwWidth_mm
	= (g_ScreenInfo[nScreenNum].dwWidth
	   / monitorResolution) * 25.4;
      g_ScreenInfo[nScreenNum].dwHeight_mm
	= (g_ScreenInfo[nScreenNum].dwHeight
	   / monitorResolution) * 25.4;

      /*
       * Keep track of the last screen number seen, as parameters seen
       * before a screen number apply to all screens, whereas parameters
       * seen after a screen number apply to that screen number only.
       */
      g_iLastScreen = nScreenNum;

      /* Keep a count of the number of screens */
      ++g_iNumScreens;

      return iArgsProcessed;
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
      if (-1 == g_iLastScreen)
	{
	  int		j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].dwEnginePreferred = dwEngine;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].dwEnginePreferred = dwEngine;
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
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].fFullScreen = TRUE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].fFullScreen = TRUE;
	}

      /* Indicate that we have processed this argument */
      return 1;
    }

  /*
   * Look for the '-ignoreinput' argument
   */
  if (strcmp(argv[i], "-ignoreinput") == 0)
    {
      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].fIgnoreInput = TRUE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].fIgnoreInput = TRUE;
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
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].iE3BTimeout = iE3BTimeout;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].iE3BTimeout = TRUE;
	}

      /* Indicate that we have processed this argument */
      return iArgsProcessed;
    }

  /*
   * Look for the '-depth n' argument
   */
  if (strcmp (argv[i], "-depth") == 0)
    {
      DWORD		dwDepth = 0;
      
      /* Display the usage message if the argument is malformed */
      if (++i >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      /* Grab the argument */
      dwDepth = atoi (argv[i]);

      /* Is this parameter attached to a screen or global? */
      if (-1 == g_iLastScreen)
	{
	  int		j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].dwDepth = dwDepth;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].dwDepth = dwDepth;
	}
      
      /* Indicate that we have processed the argument */
      return 2;
    }

  /*
   * Look for the '-refresh n' argument
   */
  if (strcmp (argv[i], "-refresh") == 0)
    {
      DWORD		dwRefreshRate = 0;
      
      /* Display the usage message if the argument is malformed */
      if (++i >= argc)
	{
	  UseMsg ();
	  return 0;
	}

      /* Grab the argument */
      dwRefreshRate = atoi (argv[i]);

      /* Is this parameter attached to a screen or global? */
      if (-1 == g_iLastScreen)
	{
	  int		j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].dwRefreshRate = dwRefreshRate;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].dwRefreshRate = dwRefreshRate;
	}
      
      /* Indicate that we have processed the argument */
      return 2;
    }

  /*
   * Look for the '-nowinkill' argument
   */
  if (strcmp(argv[i], "-nowinkill") == 0)
    {
      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].fUseWinKillKey = FALSE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].fUseWinKillKey = FALSE;
	}

      /* Indicate that we have processed this argument */
      return 1;
    }

  /*
   * Look for the '-winkill' argument
   */
  if (strcmp(argv[i], "-winkill") == 0)
    {
      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].fUseWinKillKey = TRUE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].fUseWinKillKey = TRUE;
	}

      /* Indicate that we have processed this argument */
      return 1;
    }

  /*
   * Look for the '-nounixkill' argument
   */
  if (strcmp(argv[i], "-nounixkill") == 0)
    {
      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].fUseUnixKillKey = FALSE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].fUseUnixKillKey = FALSE;
	}

      /* Indicate that we have processed this argument */
      return 1;
    }

  /*
   * Look for the '-unixkill' argument
   */
  if (strcmp(argv[i], "-unixkill") == 0)
    {
      /* Is this parameter attached to a screen or is it global? */
      if (-1 == g_iLastScreen)
	{
	  int			j;

	  /* Parameter is for all screens */
	  for (j = 0; j < MAXSCREENS; j++)
	    {
	      g_ScreenInfo[j].fUseUnixKillKey = TRUE;
	    }
	}
      else
	{
	  /* Parameter is for a single screen */
	  g_ScreenInfo[g_iLastScreen].fUseUnixKillKey = TRUE;
	}

      /* Indicate that we have processed this argument */
      return 1;
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

#if CYGDEBUG
  ErrorF ("InitOutput ()\n");
#endif

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
  for (i = 0; i < g_iNumScreens; i++)
    {
      if (-1 == AddScreen (winScreenInit, argc, argv))
	{
	  FatalError ("Couldn't add screen %d", i);
	}
    }
}
