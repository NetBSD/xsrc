/* $XFree86: xc/programs/Xserver/hw/xfree68/pm2/pm2fbdev.c,v 1.1.2.2 1999/06/02 12:08:27 hohndel Exp $ */
/*

	pm2fbdev.c - Acceleration routines for XF68_FBDev and pm2fb
	
	Michel Dänzer (michdaen@iiic.ethz.ch)
	
	Thanks to Sven Luther for the original framework
	
	Most of this code is taken from the XFree86 3.3.3.1 sources
	
*/

#include "pm2_accel.h"

#define logbytesperaccess 2
#define PARTPROD(a,b,c) (((a)<<6) | ((b)<<3) | (c))

int partprodPermedia[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,1,1), PARTPROD(1,1,1), PARTPROD(1,1,2),
	PARTPROD(1,2,2), PARTPROD(1,2,2), PARTPROD(1,2,3), PARTPROD(2,2,3),
	PARTPROD(1,3,3), PARTPROD(2,3,3),              -1, PARTPROD(3,3,3),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(3,3,4), 
	PARTPROD(1,4,4), PARTPROD(2,4,4),              -1, PARTPROD(3,4,4), 
	             -1,              -1,              -1, PARTPROD(4,4,4), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(4,4,5), 
	PARTPROD(1,5,5), PARTPROD(2,5,5),              -1, PARTPROD(3,5,5), 
	             -1,              -1,              -1, PARTPROD(4,5,5), 
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1, PARTPROD(5,5,5), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6),
	             -1,              -1,              -1, PARTPROD(4,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1, PARTPROD(5,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1};

extern pointer fbdevVirtBase, fbdevRegBase;
pointer glintVideoMem = NULL;

ScrnInfoRec glintInfoRec;

volatile pointer GLINTMMIOBase = NULL;

int Bppshift, pprod, pm2fbVirtX, pm2fbVirtY, pm2fbMaxX, pm2fbMaxY;
int pm2fbPixmapIndex;

/* Illo says it doesn't work on B/CVisions(PCIDisconnect) */
Bool UsePCIRetry = FALSE;

void (*pm2fbSetupForScreenToScreenCopy)(int xdir, int ydir, int rop, unsigned planemask, int transparency_color);
void (*pm2fbSubsequentScreenToScreenCopy)(int x1, int y1, int x2, int y2, int w, int h);


int pm2fb_reinit (ScreenPtr pScreen)
{
	/* Wait until Illo's console render madness is finished ;) */
	PM2_WAIT_IDLE();
	
	/* This assumes there's no 24 bpp */
	/* Illo, why isn't this initialized by pm2fb? ;) */
	GLINT_WAIT(2);
	GLINT_WRITE_REG((glintInfoRec.bitsPerPixel)>>4, FBReadPixel);
	/* Must be disabled for the pixmap cache */
	GLINT_WRITE_REG(UNIT_DISABLE, ScissorMode);

	pm2fbCacheInit(pm2fbMaxX, pm2fbMaxY);
}

int pm2fb_init (ScreenPtr pScreen)
{
	GLINTMMIOBase = fbdevRegBase;
	glintInfoRec = fbdevInfoRec;
	glintVideoMem = fbdevVirtBase;

	pm2fbVirtX = pm2fbMaxX = glintInfoRec.virtualX;
	pm2fbVirtY = pm2fbMaxY = glintInfoRec.virtualY;

	pprod = partprodPermedia[pm2fbMaxX >> 5];

    	/* I put this here, assuming that bpp can't change while the server runs */
    	switch (glintInfoRec.bitsPerPixel) {
    		case 8:
			Bppshift = logbytesperaccess;
			pm2fbSetupForScreenToScreenCopy =
				Permedia2SetupForScreenToScreenCopy;
			pm2fbSubsequentScreenToScreenCopy =
				Permedia2SubsequentScreenToScreenCopy;
			break;
    		case 16:
    			Bppshift = logbytesperaccess-1;    			
			pm2fbSetupForScreenToScreenCopy =
				Permedia2SetupForScreenToScreenCopy;
			pm2fbSubsequentScreenToScreenCopy =
				Permedia2SubsequentScreenToScreenCopy;
			break;
    		case 32:
			Bppshift = logbytesperaccess-2;
			pm2fbSetupForScreenToScreenCopy =
				Permedia2SetupForScreenToScreenCopy32bpp;
			pm2fbSubsequentScreenToScreenCopy =
				Permedia2SubsequentScreenToScreenCopy32bpp;
			break;
    		default:
			ErrorF("pm2fb_init: Unsupported depth: %d\n",
			        glintInfoRec.bitsPerPixel);
			return FALSE;
    	}

#ifdef DEBUG
	ErrorF("pm2fb_init: Bppshift = %d, pprod = %d\n", Bppshift, pprod);
#endif

	pm2fb_reinit (pScreen) ;

	pm2fbImageInit();
#ifdef PM2_FBDEV_SERVER_GC_ACCEL
	pm2fbInitGC();
#endif
	
#ifdef PIXPRIV
        pm2fbPixmapIndex = AllocatePixmapPrivateIndex();
        if (!AllocatePixmapPrivate(pScreen, pm2fbPixmapIndex,
			       sizeof(pm2fbPixPrivRec)))
	return FALSE;
#endif
	
	pScreen->CopyWindow = pm2fbCopyWindow ;
	pScreen->GetImage = pm2fbGetImage;
	pScreen->PaintWindowBackground = pm2fbPaintWindow;
	pScreen->PaintWindowBorder = pm2fbPaintWindow;
#ifdef PM2_FBDEV_SERVER_GC_ACCEL
	pScreen->CreateGC = pm2fbCreateGC;
#endif
  	return TRUE;
}
