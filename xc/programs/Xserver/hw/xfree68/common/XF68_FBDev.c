/* $XFree86: xc/programs/Xserver/hw/xfree68/common/XF68_FBDev.c,v 3.0 1996/08/21 13:05:22 dawes Exp $ */
/* $XConsortium: XF86_Hga.c,v 1.1 94/03/28 21:21:40 dpw Exp $ */
#include "X.h"
#include "os.h"

#define _NO_XF86_PROTOTYPES

#include "xf86.h"
#include "xf86_Config.h"


extern ScrnInfoRec fbdevInfoRec;

ScrnInfoPtr xf86Screens[] = {
	&fbdevInfoRec,
};

int xf86MaxScreens = sizeof(xf86Screens)/sizeof(ScrnInfoPtr);

int xf86ScreenNames[] = {
	FBDEV,
	-1
};

int fbdevValidTokens[] = {
	STATICGRAY,
	GRAYSCALE,
	STATICCOLOR,
	PSEUDOCOLOR,
	TRUECOLOR,
	DIRECTCOLOR,
	MODES,
	VIEWPORT,
	VIRTUAL,
	-1
};

#include "xf86ExtInit.h"
