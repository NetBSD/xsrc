/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/cygwin/cygwin_video.c,v 1.1 2000/08/10 17:40:36 dawes Exp $ */
/*
 * Copyright 1993-1999 by The XFree86 Project, Inc
 *
 */

#include "X.h"
#include "input.h"
#include "scrnintstr.h"
#include <sys/mman.h>

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86OSpriv.h"
#include "xf86_OSproc.h"

/*
 * This file contains the common part of the video memory mapping functions
 */

/*
 * Get a piece of the ScrnInfoRec.  At the moment, this is only used to hold
 * the MTRR option information, but it is likely to be expanded if we do
 * auto unmapping of memory at VT switch.
 *
 */

typedef struct {
	unsigned long	physBase;
	unsigned long	size;
	pointer		virtBase;
	pointer 	mtrrInfo;
	int		flags;
} MappingRec, *MappingPtr;
	
typedef struct {
	int		numMappings;
	MappingPtr *	mappings;
	Bool		mtrrEnabled;
	MessageType	mtrrFrom;
	Bool		mtrrOptChecked;
	ScrnInfoPtr	pScrn;
} VidMapRec, *VidMapPtr;

static int vidMapIndex = -1;

#define VIDMAPPTR(p) ((VidMapPtr)((p)->privates[vidMapIndex].ptr))

static VidMemInfo vidMemInfo = {FALSE, };

static VidMapPtr
getVidMapRec(int scrnIndex)
{
	VidMapPtr vp;

	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	if (vidMapIndex < 0)
		vidMapIndex = xf86AllocateScrnInfoPrivateIndex();

	if (VIDMAPPTR(pScrn) != NULL)
		return VIDMAPPTR(pScrn);

	vp = pScrn->privates[vidMapIndex].ptr = xnfcalloc(sizeof(VidMapRec), 1);
	vp->mtrrEnabled = TRUE;	/* default to enabled */
	vp->mtrrFrom = X_DEFAULT;
	vp->mtrrOptChecked = FALSE;
	vp->pScrn = pScrn;
	return vp;
}

static MappingPtr
newMapping(VidMapPtr vp)
{
	vp->mappings = xnfrealloc(vp->mappings, sizeof(MappingPtr) *
				  (vp->numMappings + 1));
	vp->mappings[vp->numMappings] = xnfcalloc(sizeof(MappingRec), 1);
	return vp->mappings[vp->numMappings++];
}

static MappingPtr
findMapping(VidMapPtr vp, pointer vbase, unsigned long size)
{
	int i;

	for (i = 0; i < vp->numMappings; i++) {
		if (vp->mappings[i]->virtBase == vbase &&
		    vp->mappings[i]->size == size)
			return vp->mappings[i];
	}
	return NULL;
}

static void
removeMapping(VidMapPtr vp, MappingPtr mp)
{
	int i, found = 0;

	for (i = 0; i < vp->numMappings; i++) {
		if (vp->mappings[i] == mp) {
			found = 1;
			xfree(vp->mappings[i]);
		} else if (found) {
			vp->mappings[i - 1] = vp->mappings[i];
		}
	}
	vp->numMappings--;
	vp->mappings[vp->numMappings] = NULL;
}

enum { OPTION_MTRR };
static OptionInfoRec opts[] =
{
	{ OPTION_MTRR, "mtrr", OPTV_BOOLEAN, {0}, FALSE },
	{ -1, NULL, OPTV_NONE, {0}, FALSE }
};

static void
checkMtrrOption(VidMapPtr vp)
{
	if (!vp->mtrrOptChecked && vp->pScrn->options != NULL) {
		/*
		 * We get called once for each screen, so reset
		 * the OptionInfoRecs.
		 */
		opts[0].found = FALSE;

		xf86ProcessOptions(vp->pScrn->scrnIndex, vp->pScrn->options,
					opts);
		if (xf86GetOptValBool(opts, OPTION_MTRR, &vp->mtrrEnabled))
			vp->mtrrFrom = X_CONFIG;
		vp->mtrrOptChecked = TRUE;
	}
}


/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool xf86DisableInterrupts()
{
	/* allow interrupt disabling but check for side-effects. 
	 * Not a good policy ...
	 */
        __asm__ __volatile__("sti");
	return TRUE;
}

void xf86EnableInterrupts()
{
	/*Reenable*/
        __asm__ __volatile__ ("sti");
}



/***************************************************************************/
/* Initialize video memory                                                 */
/***************************************************************************/

void xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
	return 0;
}
