/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/SC11412.h,v 1.1.1.2 1996/01/03 07:19:50 dawes Exp $ */


/* Norbert Distler ndistler@physik.tu-muenchen.de */


/* $XConsortium: SC11412.h /main/2 1995/11/12 19:30:24 kaleb $ */

typedef int Bool;
#define TRUE 1
#define FALSE 0
#define QUARZFREQ	        14318
#define MIN_SC11412_FREQ        45000
#define MAX_SC11412_FREQ       100000

Bool SC11412SetClock( 
#if NeedFunctionPrototypes
   long
#endif
);     
