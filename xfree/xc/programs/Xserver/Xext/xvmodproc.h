/* $XFree86: xc/programs/Xserver/Xext/xvmodproc.h,v 1.1 1998/08/13 14:45:36 dawes Exp $ */

extern int (*XvGetScreenIndexProc)(void);
extern unsigned long (*XvGetRTPortProc)(void);
extern int (*XvScreenInitProc)(ScreenPtr);

extern void XvRegister(void);
