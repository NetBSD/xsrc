/* $TOG: pm.h /main/2 1997/11/10 16:08:00 barstow $ */
/*

Copyright "1986-1997 The Open Group All Rights Reserved

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to use the Software without restriction,
including, without limitation, the rights to copy, modify,
merge, publish, distribute and sublicense the Software, to make,
have made, license and distribute derivative works thereof, and
to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and the following permission notice
shall be included in all copies of the Software:

THE SOFTWARE IS PROVIDED "AS IS ", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NON-INFRINGEMENT. IN NO EVENT SHALL THE OPEN GROUP BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER SIABILITIY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN
CONNNECTION WITH THE SOFTWARE OR THE USE OF OTHER DEALINGS IN
THE SOFTWARE.

Except as contained in this notice, the name of The Open Group
shall not be used in advertising or otherwise to promote the use
or other dealings in this Software without prior written
authorization from The Open Group.

X Window System is a trademark of The Open Group.

*/
/* $XFree86: xc/programs/xfwp/pm.h,v 1.3 1999/03/02 11:49:40 dawes Exp $ */

#ifndef _PM_H
#define _PM_H

extern void FWPprocessMessages(
    IceConn iceConn,
    IcePointer * client_data,
    int opcode,
    unsigned long length,
    Bool swap);

extern Bool
FWPHostBasedAuthProc (
    char * hostname);

extern Status
FWPprotocolSetupProc(
    IceConn iceConn,
    int major_version,
    int minor_version,
    char * vendor,
    char * release,
    IcePointer * clientDataRet,
    char ** failureReasonRet);

extern int 
doSetupPMListen(
    char *  pm_port,
    int * size_pm_listen_array,
    int ** pm_listen_array,
    IceListenObj ** listen_objects,
    int * nfds,
    fd_set * rinit);

extern void 
doInstallIOErrorHandler (void);

#endif /* _PM_H */
