/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/rendition/vmisc.h,v 1.2 1999/11/19 14:59:18 hohndel Exp $ */

#ifndef __VMISC_H__
#define __VMISC_H__

#include "rendition.h"
#include "vtypes.h"
#include "vos.h"

void verite_bustomem_cpy (vu8 *, vu8 *, vu32);
void verite_memtobus_cpy (vu8 *, vu8 *, vu32);

#endif /* __VMISC_H__ */
