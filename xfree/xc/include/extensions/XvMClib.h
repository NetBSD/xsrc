/* $XFree86: xc/include/extensions/XvMClib.h,v 1.3 2001/04/01 13:59:59 tsi Exp $ */

#ifndef _XVMCLIB_H_
#define _XVMCLIB_H_

#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XvMC.h>

Bool XvMCQueryExtension (Display *display, int *eventBase, int *errBase);
Status XvMCQueryVersion (Display *display, int *major, int *minor);

XvMCSurfaceInfo * XvMCListSurfaceTypes(Display *dpy, XvPortID port, int *num);

Status XvMCCreateContext (
   Display *display,
   XvPortID port,
   int surface_type_id,
   int width,
   int height,
   int flags,
   XvMCContext * context
);

Status XvMCDestroyContext (Display *display, XvMCContext * context);

Status 
XvMCCreateSurface(
  Display *display,
  XvMCContext * context,
  XvMCSurface * surface
);

Status XvMCDestroySurface(Display *display, XvMCSurface *surface);

XvImageFormatValues * XvMCListSubpictureTypes (
  Display * display,
  XvPortID port,
  int surface_type_id,
  int *count_return
);

Status
XvMCPutSurface(
  Display *display,
  XvMCSurface *surface,
  Drawable draw,
  short srcx, 
  short srcy, 
  unsigned short srcw, 
  unsigned short srch,
  short destx,
  short desty,
  unsigned short destw,
  unsigned short desth,
  int flags
);

Status XvMCHideSurface(Display *display, XvMCSurface *surface);

Status
XvMCCreateSubpicture (
   Display *display, 
   XvMCContext *context,
   XvMCSubpicture *subpicture, 
   unsigned short width,
   unsigned short height,
   int xvimage_id
);


Status
XvMCClearSubpicture (
  Display *display,
  XvMCSubpicture *subpicture,
  short x,
  short y,
  unsigned short width,
  unsigned short height,
  unsigned int color
);

Status
XvMCCompositeSubpicture (
   Display *display,
   XvMCSubpicture *subpicture,
   XvImage *image,
   short srcx,
   short srcy,
   unsigned short width,
   unsigned short height,
   short dstx,
   short dsty
);

Status
XvMCDestroySubpicture (Display *display, XvMCSubpicture *subpicture);

Status
XvMCSetSubpicturePalette (
  Display *display, 
  XvMCSubpicture *subpicture, 
  unsigned char *palette
);

Status
XvMCBlendSubpicture (
   Display *display,
   XvMCSurface *target_surface,
   XvMCSubpicture *subpicture,
   short subx,
   short suby,
   unsigned short subw,
   unsigned short subh,
   short surfx,
   short surfy,
   unsigned short surfw,
   unsigned short surfh
);

Status
XvMCBlendSubpicture2 (
   Display *display,
   XvMCSurface *source_surface,
   XvMCSurface *target_surface,
   XvMCSubpicture *subpicture,
   short subx,
   short suby,
   unsigned short subw,
   unsigned short subh,
   short surfx,
   short surfy,
   unsigned short surfw,
   unsigned short surfh
);

Status XvMCSyncSurface (Display *display, XvMCSurface *surface);
Status XvMCFlushSurface (Display *display, XvMCSurface *surface);
Status XvMCGetSurfaceStatus (Display *display, XvMCSurface *surface, int *stat);

Status XvMCSyncSubpicture (Display *display, XvMCSubpicture *subpicture);
Status XvMCFlushSubpicture (Display *display, XvMCSubpicture *subpicture);
Status
XvMCGetSubpictureStatus (Display *display, XvMCSubpicture *subpic, int *stat);

#endif
