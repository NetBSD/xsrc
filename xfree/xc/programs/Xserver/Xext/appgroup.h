/* $XFree86: xc/programs/Xserver/Xext/appgroup.h,v 1.2 2004/06/25 15:44:42 tsi Exp $ */

#ifndef APPGROUP_H
#define APPGROUP_H 1

void XagClientStateChange(
    CallbackListPtr* pcbl,
    pointer nulldata,
    pointer calldata);
int ProcXagCreate (
    register ClientPtr client);
int ProcXagDestroy(
    register ClientPtr client);

#endif
