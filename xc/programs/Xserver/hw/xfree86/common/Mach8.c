/* $XFree86: xc/programs/Xserver/hw/xfree86/common/Mach8.c,v 3.4 1996/02/04 09:05:50 dawes Exp $ */





/* $XConsortium: Mach8.c /main/2 1995/11/12 19:20:37 kaleb $ */
#include "X.h"
#include "os.h"

#define _NO_XF86_PROTOTYPES

#include "xf86.h"
#include "xf86_Config.h"

extern ScrnInfoRec mach8InfoRec;

/*
 * This limit is set to a value which is typical for many of the ramdacs
 * used on Mach8 cards.  Increasing this limit could result in damage to
 * to your hardware.
 */
/* XXXX This value needs to be checked (currently using 80MHz) */
#define MAX_MACH8_CLOCK 80000

int mach8MaxClock = MAX_MACH8_CLOCK;

ScrnInfoPtr xf86Screens[] = 
{
  &mach8InfoRec,
};

int  xf86MaxScreens = sizeof(xf86Screens) / sizeof(ScrnInfoPtr);

int xf86ScreenNames[] =
{
  ACCEL,
  -1
};

int mach8ValidTokens[] =
{
  STATICGRAY,
  GRAYSCALE,
  STATICCOLOR,
  PSEUDOCOLOR,
  TRUECOLOR,
  DIRECTCOLOR,
  CHIPSET,
  CLOCKS,
  MODES,
  OPTION,
  VIDEORAM,
  VIEWPORT,
  VIRTUAL,
  CLOCKPROG,
  BIOSBASE,
  -1
};

#include "xf86ExtInit.h"

