/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/rendition/vmodes.h,v 1.4 2000/03/31 20:13:27 dawes Exp $ */
/*
 * file vmodes.h
 *
 * headerfile for vmodes.c
 */

#ifndef __VMODES_H__
#define __VMODES_H__



/*
 * includes
 */

#include "vtypes.h"



/*
 * function prototypes
 */

int verite_setmodefixed(ScrnInfoPtr pScreenInfo);
int verite_setmode(ScrnInfoPtr pScreenInfo, struct verite_modeinfo_t *mode);
void verite_setframebase(ScrnInfoPtr pScreenInfo, vu32 framebase);
int verite_getstride(ScrnInfoPtr pScreenInfo, int *width, vu16 *stride0, vu16 *stride1);



#endif /* #ifndef _VMODES_H_ */

/*
 * end of file vmodes.h
 */
