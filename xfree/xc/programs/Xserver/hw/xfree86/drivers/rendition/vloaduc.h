/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/rendition/vloaduc.h,v 1.3 1999/10/13 04:21:23 dawes Exp $ */

/*
 * file vloaduc.h
 *
 * loads microcode
 */

#ifndef __VLOADUC_H__
#define __VLOADUC_H__



/*
 * includes
 */

#include "vos.h"
#include "vtypes.h"



/*
 * defines 
 */



/*
 * function prototypes
 */

int verite_load_ucfile(ScrnInfoPtr pScreenInfo, char *file_name);



#endif /* __VLOADUC_H__ */

/*
 * end of file vloaduc.h
 */
