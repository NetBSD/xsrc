/* $XFree86: xc/programs/Xserver/hw/xfree86/xf86Version.h,v 3.236.2.76 1999/01/04 19:16:05 hohndel Exp $ */

#define XF86_VERSION " 3.3.3.1 "

/* The finer points in versions... */
#define XF86_VERSION_MAJOR	3
#define XF86_VERSION_MINOR	3
#define XF86_VERSION_SUBMINOR	3
#define XF86_VERSION_PATCHLEV	1
#define XF86_VERSION_BETA	XF86_VERSION_PATCHLEV
#define XF86_VERSION_ALPHA	0	/* 0="", 1="a", 2="b", etc... */

#define XF86_VERSION_NUMERIC(major,minor,subminor,beta,alpha)	\
   ((((((((major << 7) | minor) << 7) | subminor) << 5) | beta) << 5) | alpha)
#define XF86_VERSION_CURRENT					\
   XF86_VERSION_NUMERIC(XF86_VERSION_MAJOR,			\
			XF86_VERSION_MINOR,			\
			XF86_VERSION_SUBMINOR,			\
			XF86_VERSION_BETA,			\
			XF86_VERSION_ALPHA)

#define XF86_DATE	"January 4 1999"

/* $XConsortium: xf86Version.h /main/78 1996/10/28 05:42:10 kaleb $ */
