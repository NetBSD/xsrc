/*-
 * amigaC.c --
 *	Functions for handling the amiga BWTWO board.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */

/************************************************************
Copyright (c) 1995 by Daniver Limited (Gary Henderson)
Copyright 1994 by Eduardo Horvath
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifdef AMIGA_CC_COLOR

#ifndef	lint
static char sccsid[] = "%W %G Copyright 1987 Sun Micro";
#endif

/*-
 * Copyright (c) 1987 by Sun Microsystems,  Inc.
 */

#include    "amiga.h"
#include    "afb.h"
#include    "resource.h"
#include "miline.h"
    
#define GXZEROLINEBIAS	(OCTANT1 | OCTANT3 | OCTANT4 | OCTANT6)

#include    <sys/mman.h>

#if defined(__NetBSD__) && !defined(MAP_FILE)
#define MAP_FILE 0
#endif


extern void *mmap();
static void amigaCInstallColormap ();
static void amigaCUninstallColormap ();
static int amigaCListInstalledColormaps ();
static void amigaCStoreColors ();
static void  amigaCUpdateColormap ();

/* These are set by ddxProcessArgument */
int amigaCCWidth = 700;
int amigaCCHeight = 430;
int amigaCCDepth = 4;
int numColors = 16;
int amigaCCXOffset;
int amigaCCYOffset;

/*-
 *-----------------------------------------------------------------------
 * amigaCInit --
 *	Attempt to find and initialize a bw2 framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Most of the elements of the ScreenRec are filled in.  The
 *	video is enabled for the frame buffer...
 *
 *-----------------------------------------------------------------------
 */
#define StaticGrayMask	(1 << StaticGray)
#define GrayScaleMask	(1 << GrayScale)
#define StaticColorMask	(1 << StaticColor)
#define PseudoColorMask	(1 << PseudoColor)
#define TrueColorMask	(1 << TrueColor)
#define DirectColorMask (1 << DirectColor)

#define ALL_VISUALS	(StaticGrayMask|\
			 GrayScaleMask|\
			 StaticColorMask|\
			 PseudoColorMask|\
			 TrueColorMask|\
			 DirectColorMask)
 
/*ARGSUSED*/
Bool
amigaCInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    amigaScreenPtr pPrivate;
    unsigned long visuals = 0;
    colormap_t *colormap = &amigaFbs[index].view.colormap;

    if (colormap->type != CM_COLOR)
    {
	if (colormap->type == CM_MONO)
	    visuals = StaticGrayMask;
	else /* CM_GREYSCALE */
	    visuals = StaticGrayMask | GrayScaleMask;
    }
    else
    switch (amigaCCDepth)
    {
    case 1:
	visuals = StaticGrayMask;
	break;
    case 2:
    case 3:
    case 4:
    case 5:
	visuals = StaticGrayMask | GrayScaleMask | StaticColorMask |
		  PseudoColorMask;
	break;

    default:
        visuals = ALL_VISUALS;
	break;
    }

    if (!afbSetVisualTypes (amigaCCDepth, visuals,
			    colormap->red_mask == 0xff ? 8 : 4))
    {
	ErrorF ("afbSetVisualTypes: FALSE\n");
	return (FALSE);
    }

    if (!afbScreenInit(pScreen,
		       amigaFbs[index].fb,
		       amigaFbs[index].view.vs.width,
		       amigaFbs[index].view.vs.height,
		       monitorResolution, monitorResolution,
		       amigaFbs[index].view.vs.width))
    {
        ErrorF ("afbScreenInit: FALSE\n");
	return (FALSE);
    }

    pScreen->InstallColormap = amigaCInstallColormap;
    pScreen->UninstallColormap = amigaCUninstallColormap;
    pScreen->ListInstalledColormaps = amigaCListInstalledColormaps;
    pScreen->StoreColors = amigaCStoreColors;
    
    if (!amigaScreenAllocate (pScreen))
    {
	ErrorF ("amigaScreenAllocate: FALSE\n");
	return (FALSE);
    }

    if (!amigaScreenInit (pScreen))
    {
	ErrorF ("amigaScreenInit: FALSE\n");
	return FALSE;
    }

    pPrivate = (amigaScreenPtr) pScreen->devPrivates[amigaScreenIndex].ptr;
    pPrivate->UpdateColormap = amigaCUpdateColormap;

    /*
     * Enable video output...? 
     */
    (void) amigaSaveScreen(pScreen, SCREEN_SAVER_OFF);

    if (!afbCreateDefColormap (pScreen))
    {
	ErrorF ("afbCreateDefColormap: FALSE\n");
	return (FALSE);
    }

    miSetZeroLineBias(pScreen, GXZEROLINEBIAS);

    return TRUE;
}
/*-
 *-----------------------------------------------------------------------
 * xopen_view (void) --
 *	Scan for and initialize a view to use.
 *
 * Results:
 *	Returns fd of opened view.
 *
 * Side Effects:
 *	none
 *
 *-----------------------------------------------------------------------
 */
int
xopen_view (void)
{
    u_char buffer[13];
    int i, fd;
    
    for (i=0; i < 100; i++) {
	sprintf (buffer, "/dev/view%02d", i);
	fd = open (buffer, O_RDWR);
	if (fd < 0 && errno != EBUSY) {
	    perror ("xopen_view()");
	    return (-1);
	} else if (fd >= 0) {
	    return (fd);
	}
    }
    Error ("Ran out of views");
    return (-1);
}

/*-
 *-----------------------------------------------------------------------
 * amigaCProbe --
 *	Attempt to find and initialize a bw2 framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped. 
 *
 *-----------------------------------------------------------------------
 */

/*ARGSUSED*/
Bool
amigaCProbe(pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the amigaFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         fd;
    int		pagemask, mapsize;
    caddr_t	addr, mapaddr;
    struct view_size vs;
    bmap_t	bm;
    colormap_t	colormap;
    
    if ((fd = xopen_view()) < 0)
	return FALSE;

    if (ioctl(fd, VIOCGSIZE, &vs)) {
        Error("ioctl VIOCGSIZE");
        return FALSE;
      }

    vs.width = amigaCCWidth;
    vs.height = amigaCCHeight;
    vs.depth = amigaCCDepth;
    vs.x = amigaCCXOffset;
    vs.y = amigaCCYOffset;

    if (ioctl(fd, VIOCSSIZE, &vs)) {
        Error("ioctl VIOCGSIZE");
        return FALSE;
      }

    if (ioctl(fd, VIOCGSIZE, &vs)) {
        Error("ioctl VIOCGSIZE");
        return FALSE;
      }

    amigaCCDepth = vs.depth;
    numColors = 1 << amigaCCDepth;
    
    if (ioctl(fd, VIOCGBMAP, &bm)) {
        Error ("ioctl VIOCGBMAP");
        return FALSE;
      }

    if ((mapaddr = (caddr_t) mmap(0, bm.bytes_per_row*bm.rows*bm.depth,
        PROT_READ | PROT_WRITE, MAP_FILE, fd, (off_t)0)) == (caddr_t) -1) {
        Error("mmapping bitmap");
        (void) close(fd);
        return FALSE;
      }

    if (mapaddr) {
	unsigned long endaddr, guardaddr, pagesize;
	caddr_t guardp, guardp1;

	pagesize = getpagesize();

	endaddr = (unsigned long)mapaddr + bm.bytes_per_row*bm.rows*bm.depth-1;
	guardaddr = endaddr + 8;
	if ((guardaddr ^ endaddr) & ~(pagesize-1)) {
		/* need guard page */
		guardp = (caddr_t)(guardaddr & ~(pagesize-1));
		guardp1 = mmap(guardp, pagesize, PROT_READ|PROT_WRITE,
		    MAP_ANON|MAP_FIXED, -1, 0);
		if (guardp1 == (caddr_t)-1) {
			Error("Can't allocate guard page");
		}
	}
    }

    if (mapaddr == 0)
        mapaddr = addr;

    colormap.first = 0;
    colormap.size = numColors;
    colormap.entry = amigaFbs[fbNum].view.entry;
    
    if(ioctl (fd, VIOCGCMAP, &colormap)== -1)
        Error("getting colormap");

    amigaFbs[fbNum].fb = (pointer)mapaddr;
    amigaFbs[fbNum].fd = fd;
    amigaFbs[fbNum].view.bm = bm;
    amigaFbs[fbNum].view.vs = vs;
    amigaFbs[fbNum].view.colormap = colormap;
    amigaFbs[fbNum].EnterLeave = NULL;

#ifndef GDBUG
    if (ioctl(fd, VIOCDISPLAY, 0)) {
        Error ("ioctl VIOCDISPLAY");
        return FALSE;
      }
#endif

    return TRUE;
}

Bool
amigaCCreate(pScreenInfo, argc, argv)
    ScreenInfo	  *pScreenInfo;
    int	    	  argc;
    char    	  **argv;
{
    return (AddScreen(amigaCInit, argc, argv) >= 0);
}

static void
amigaCUpdateColormap(pScreen, index, count, rmap, gmap, bmap)
     ScreenPtr	pScreen;
     int	index, count;
     u_char	*rmap, *gmap, *bmap;
{
    colormap_t *colormap = &amigaFbs[pScreen->myNum].view.colormap;
    
    while (count--)
    {
	colormap->entry [index]= (rmap[index] << 16) | (gmap[index] << 8) |
				  bmap[index] ;
	index++;
    }
    if (ioctl (amigaFbs [pScreen->myNum].fd, VIOCSCMAP, colormap) == -1)
        Error ("Setting colormap");
}


/*-
 *-----------------------------------------------------------------------
 * amigaCInstallColormap --
 *	Install given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Existing map is uninstalled.
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
amigaCInstallColormap(cmap)
    ColormapPtr	cmap;
{
    SetupScreen(cmap->pScreen);
    register int i;
    register Entry *pent;
    register VisualPtr pVisual = cmap->pVisual;
    register int bitshift = 8;
    u_char	  rmap[MAX_COLORS], gmap[MAX_COLORS], bmap[MAX_COLORS];
  
    if (cmap == pPrivate->installedMap)
        return;

    bitshift = 16 - cmap->pVisual->bitsPerRGBValue;
    
  if (pPrivate->installedMap)
    WalkTree(pPrivate->installedMap->pScreen, TellLostMap,
	     (pointer) &(pPrivate->installedMap->mid));
  if ((pVisual->class | DynamicClass) == DirectColor) {
    for (i = 0; i < numColors; i++) {
      pent = &cmap->red[(i & pVisual->redMask) >>
			pVisual->offsetRed];
      rmap[i] = pent->co.local.red >> bitshift;
      pent = &cmap->green[(i & pVisual->greenMask) >>
			  pVisual->offsetGreen];
      gmap[i] = pent->co.local.green >> bitshift;
      pent = &cmap->blue[(i & pVisual->blueMask) >>
			 pVisual->offsetBlue];
      bmap[i] = pent->co.local.blue >> bitshift;
    }
  } else {
    for (i = 0, pent = cmap->red;
	 i < pVisual->ColormapEntries;
	 i++, pent++) {
      if (pent->fShared) {
	rmap[i] = pent->co.shco.red->color >> bitshift;
	gmap[i] = pent->co.shco.green->color >> bitshift;
	bmap[i] = pent->co.shco.blue->color >> bitshift;
      }
      else {
	rmap[i] = pent->co.local.red >> bitshift;
	gmap[i] = pent->co.local.green >> bitshift;
	bmap[i] = pent->co.local.blue >> bitshift;
      }
    }
  }
  pPrivate->installedMap = cmap;
  (*pPrivate->UpdateColormap) (cmap->pScreen, 0, numColors, rmap, gmap, bmap);
  WalkTree(cmap->pScreen, TellGainedMap, (pointer) &(cmap->mid));
}

/*-
 *-----------------------------------------------------------------------
 * amigaCUninstallColormap --
 *	Uninstall given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	default map is installed
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
amigaCUninstallColormap(cmap)
    ColormapPtr	cmap;
{
  SetupScreen(cmap->pScreen);
  if (cmap == pPrivate->installedMap) {
    Colormap defMapID = cmap->pScreen->defColormap;
    
    if (cmap->mid != defMapID) {
      ColormapPtr defMap = (ColormapPtr) LookupIDByType(defMapID,
							RT_COLORMAP);
      
      if (defMap)
	(*cmap->pScreen->InstallColormap)(defMap);
      else
	ErrorF("amigaC: Can't find default colormap\n");
    }
  }
}

/*-
 *-----------------------------------------------------------------------
 * amigaCListInstalledColormaps --
 *	Fills in the list with the IDs of the installed maps
 *
 * Results:
 *	Returns the number of IDs in the list
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
amigaCListInstalledColormaps(pScreen, pCmapList)
     ScreenPtr	pScreen;
     Colormap	*pCmapList;
{
    SetupScreen(pScreen);
    *pCmapList = pPrivate->installedMap->mid;
    return (1);
  }


/*-
 *-----------------------------------------------------------------------
 * amigaCStoreColors --
 *	Sets the pixels in pdefs into the specified map.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
static void
amigaCStoreColors(pmap, ndef, pdefs)
     ColormapPtr	pmap;
     int		ndef;
     xColorItem	*pdefs;
{
  SetupScreen(pmap->pScreen);
  u_char	rmap[MAX_COLORS], gmap[MAX_COLORS], bmap[MAX_COLORS];
  xColorItem	expanddefs[MAX_COLORS];
  register int i;
  register int first = -256;
  register int priv = -256;
  register int count = 0;
  register int bitshift = 8;
  
  if (pmap != pPrivate->installedMap)
      return;
  if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
      ndef = afbExpandDirectColors(pmap, ndef, pdefs, expanddefs);
      pdefs = expanddefs;
  }

  if (pmap->pVisual->redMask == 0xf)
  bitshift = 16 - pmap->pVisual->bitsPerRGBValue;
    
  while (ndef--) {
    i = pdefs->pixel;

    if (i != priv + 1)
    {
	if (count)
	    (*pPrivate->UpdateColormap) (pmap->pScreen, first, count, rmap,
					 gmap, bmap);
	first = i;
	count = 0;
    }
    priv = i;
    rmap[i] = pdefs->red >> bitshift;
    gmap[i] = pdefs->green >> bitshift;
    bmap[i] = pdefs->blue >> bitshift;
    pdefs++;
    count++;
  }
  if (count)
    (*pPrivate->UpdateColormap) (pmap->pScreen, first, count, rmap, gmap,
				 bmap);
}
#endif /* AMIGA_CC_COLOR */
