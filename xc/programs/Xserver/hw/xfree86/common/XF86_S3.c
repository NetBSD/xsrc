/* $XFree86: xc/programs/Xserver/hw/xfree86/common/XF86_S3.c,v 3.10 1996/02/04 09:05:56 dawes Exp $ */





/* $XConsortium: XF86_S3.c /main/4 1995/11/12 19:20:49 kaleb $ */
#include "X.h"
#include "os.h"

#define _NO_XF86_PROTOTYPES

#include "xf86.h"
#include "xf86_Config.h"

extern ScrnInfoRec s3InfoRec;

/*
 * This limit is set to 110MHz because this is the limit for
 * the ramdacs used on many S3 cards Increasing this limit
 * could result in damage to your hardware.
 */
/* Clock limit for non-Bt485, non-Ti3020, non-ATT498 cards */
#define MAX_S3_CLOCK    110000

int s3MaxClock = MAX_S3_CLOCK;

ScrnInfoPtr xf86Screens[] = 
{
  &s3InfoRec,
};

int  xf86MaxScreens = sizeof(xf86Screens) / sizeof(ScrnInfoPtr);

int xf86ScreenNames[] =
{
  ACCEL,
  -1
};

int s3ValidTokens[] =
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
  MEMBASE,
  RAMDAC,
  DACSPEED,
  -1
};

#include "xf86ExtInit.h"

