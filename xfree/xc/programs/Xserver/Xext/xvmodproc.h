/* $XFree86: xc/programs/Xserver/Xext/xvmodproc.h,v 1.4 2004/06/25 15:44:42 tsi Exp $ */

#ifndef XVMODPROC_H
#define XVMODPROC_H 1

#include "xvmcext.h"

extern int (*XvGetScreenIndexProc)(void);
extern unsigned long (*XvGetRTPortProc)(void);
extern int (*XvScreenInitProc)(ScreenPtr);
extern int (*XvMCScreenInitProc)(ScreenPtr, int, XvMCAdaptorPtr);

extern void XvRegister(void);

#endif
