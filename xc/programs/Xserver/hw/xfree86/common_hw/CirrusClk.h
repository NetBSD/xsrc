/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/CirrusClk.h,v 3.3 1996/09/29 13:36:30 dawes Exp $ */





/* $XConsortium: CirrusClk.h /main/3 1995/11/12 19:29:50 kaleb $ */

int CirrusFindClock( 
#if NeedFunctionPrototypes
   int freq, int max_clock, int *num_out, int *den_out, int *usemclk_out
#endif
);     

int CirrusSetClock(
#if NeedFunctionPrototypes
   int freq
#endif
);
