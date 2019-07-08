/*
 * Copyright © 2008 Red Hat, Inc.
 * Copyright © 2010-2015 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@redhat.com)
 *   Modified for VDPAU by Aaron Plattner (aplattner@nvidia.com)
 *   and José Hiram Soltren (jsoltren@nvidia.com)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/dri2proto.h>
#include "mesa_dri2.h"
#include "util.h"

static char dri2ExtensionName[] = DRI2_NAME;
static XExtensionInfo *dri2Info;

static /* const */ XExtensionHooks dri2ExtensionHooks = {
  NULL,                   /* create_gc */
  NULL,                   /* copy_gc */
  NULL,                   /* flush_gc */
  NULL,                   /* free_gc */
  NULL,                   /* create_font */
  NULL,                   /* free_font */
  NULL,                   /* close_display */
  NULL,                   /* wire_to_event */
  NULL,                   /* event_to_wire */
  NULL,                   /* error */
  NULL,                   /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (DRI2FindDisplay,
                                   dri2Info,
                                   dri2ExtensionName,
                                   &dri2ExtensionHooks,
                                   0, NULL)

Bool
_vdp_DRI2QueryExtension(Display * dpy, int *eventBase, int *errorBase)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);

   if (XextHasExtension(info)) {
      *eventBase = info->codes->first_event;
      *errorBase = info->codes->first_error;
      return True;
   }

   if (dri2Info) {
      if (info) {
         XextRemoveDisplay(dri2Info, dpy);
      }
      XextDestroyExtension(dri2Info);
      dri2Info = NULL;
   }

   return False;
}

Bool
_vdp_DRI2QueryVersion(Display * dpy, int *major, int *minor)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2QueryVersionReply rep;
   xDRI2QueryVersionReq *req;

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   LockDisplay(dpy);
   GetReq(DRI2QueryVersion, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2QueryVersion;
   req->majorVersion = DRI2_MAJOR;
   req->minorVersion = DRI2_MINOR;
   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }
   *major = rep.majorVersion;
   *minor = rep.minorVersion;
   UnlockDisplay(dpy);
   SyncHandle();

   return True;
}

Bool
_vdp_DRI2Connect(Display * dpy, XID window, char **driverName, char **deviceName)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);
   xDRI2ConnectReply rep;
   xDRI2ConnectReq *req;

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   LockDisplay(dpy);
   GetReq(DRI2Connect, req);
   req->reqType = info->codes->major_opcode;
   req->dri2ReqType = X_DRI2Connect;
   req->window = window;
   req->driverType = DRI2DriverVDPAU;
#ifdef DRI2DriverPrimeShift
   {
      char *prime = secure_getenv("DRI_PRIME");
      if (prime) {
         unsigned int primeid;
         errno = 0;
         primeid = strtoul(prime, NULL, 0);
         if (errno == 0)
            req->driverType |=
               ((primeid & DRI2DriverPrimeMask) << DRI2DriverPrimeShift);
      }
   }
#endif

   if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }

   if (rep.driverNameLength == 0 && rep.deviceNameLength == 0) {
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }

   *driverName = Xmalloc(rep.driverNameLength + 1);
   if (*driverName == NULL) {
      _XEatData(dpy,
                ((rep.driverNameLength + 3) & ~3) +
                ((rep.deviceNameLength + 3) & ~3));
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }
   _XReadPad(dpy, *driverName, rep.driverNameLength);
   (*driverName)[rep.driverNameLength] = '\0';

   *deviceName = Xmalloc(rep.deviceNameLength + 1);
   if (*deviceName == NULL) {
      Xfree(*driverName);
      _XEatData(dpy, ((rep.deviceNameLength + 3) & ~3));
      UnlockDisplay(dpy);
      SyncHandle();
      return False;
   }
   _XReadPad(dpy, *deviceName, rep.deviceNameLength);
   (*deviceName)[rep.deviceNameLength] = '\0';

   UnlockDisplay(dpy);
   SyncHandle();

   return True;
}

void
_vdp_DRI2RemoveExtension(Display * dpy)
{
   XextRemoveDisplay(dri2Info, dpy);
   XextDestroyExtension(dri2Info);
   dri2Info = NULL;
}
