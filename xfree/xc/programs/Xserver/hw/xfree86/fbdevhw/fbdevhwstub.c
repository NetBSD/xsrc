/* $XFree86: xc/programs/Xserver/hw/xfree86/fbdevhw/fbdevhwstub.c,v 1.9 2000/10/20 14:59:01 alanh Exp $ */

#include "xf86.h"
#include "xf86cmap.h"
#include "fbdevhw.h"

/* Stubs for the static server on platforms that don't support fbdev */


Bool
fbdevHWGetRec(ScrnInfoPtr pScrn)
{
	return FALSE;
}

void
fbdevHWFreeRec(ScrnInfoPtr pScrn)
{
}


Bool
fbdevHWProbe(pciVideoPtr pPci, char *device, char **namep)
{
	return FALSE;
}

Bool
fbdevHWInit(ScrnInfoPtr pScrn, pciVideoPtr pPci, char *device)
{
	xf86Msg(X_ERROR, "fbdevhw is not available on this platform\n");
	return FALSE;
}

char*
fbdevHWGetName(ScrnInfoPtr pScrn)
{
	return NULL;
}

int
fbdevHWGetDepth(ScrnInfoPtr pScrn)
{
	return -1;
}

int
fbdevHWGetLineLength(ScrnInfoPtr pScrn)
{
}

int
fbdevHWGetType(ScrnInfoPtr pScrn)
{
	return -1;
}

int
fbdevHWGetVidmem(ScrnInfoPtr pScrn)
{
	return -1;
}

void
fbdevHWSetVideoModes(ScrnInfoPtr pScrn)
{
}

DisplayModePtr
fbdevHWGetBuildinMode(ScrnInfoPtr pScrn)
{
	return NULL;
}

void
fbdevHWUseBuildinMode(ScrnInfoPtr pScrn)
{
}

void*
fbdevHWMapVidmem(ScrnInfoPtr pScrn)
{
	return NULL;
}

int
fbdevHWLinearOffset(ScrnInfoPtr pScrn)
{
	return 0;
}

Bool
fbdevHWUnmapVidmem(ScrnInfoPtr pScrn)
{
	return FALSE;
}

void*
fbdevHWMapMMIO(ScrnInfoPtr pScrn)
{
	return NULL;
}

Bool
fbdevHWUnmapMMIO(ScrnInfoPtr pScrn)
{
	return FALSE;
}

Bool
fbdevHWModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{	
	return FALSE;
}

void
fbdevHWSave(ScrnInfoPtr pScrn)
{
}

void
fbdevHWRestore(ScrnInfoPtr pScrn)
{
}

void
fbdevHWLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
		 LOCO *colors, VisualPtr pVisual)
{
}

int
fbdevHWValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
	return MODE_ERROR;
}

Bool
fbdevHWSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
	return FALSE;
}

void
fbdevHWAdjustFrame(int scrnIndex, int x, int y, int flags)
{
}

Bool
fbdevHWEnterVT(int scrnIndex, int flags)
{
	return FALSE;
}

void
fbdevHWLeaveVT(int scrnIndex, int flags)
{
}

void
fbdevHWDPMSSet(ScrnInfoPtr pScrn, int mode, int flags)
{
}
