/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/mga/mga_map.h,v 1.1 1997/03/06 23:16:01 hohndel Exp $ */

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define CATNAME(prefix,subname) prefix##subname
#else
#define CATNAME(prefix,subname) prefix/**/subname
#endif

#if PSZ == 8
#define MGANAME(subname) CATNAME(Mga8,subname)
#elif PSZ == 16
#define MGANAME(subname) CATNAME(Mga16,subname)
#elif PSZ == 24
#define MGANAME(subname) CATNAME(Mga24,subname)
#elif PSZ == 32
#define MGANAME(subname) CATNAME(Mga32,subname)
#endif
