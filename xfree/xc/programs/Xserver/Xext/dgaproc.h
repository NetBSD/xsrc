/* $XFree86: xc/programs/Xserver/Xext/dgaproc.h,v 1.23 2004/12/07 15:59:16 tsi Exp $ */

#ifndef __DGAPROC_H
#define __DGAPROC_H

#undef _XF86DGA_SERVER_
#define _XF86DGA_SERVER_
#include <X11/extensions/xf86dga.h>
#include <X11/Xproto.h>
#include "pixmap.h"

/* Compatibility #define's */
#define DGA_CONCURRENT_ACCESS	XDGAConcurrentAccess
#define DGA_FILL_RECT		XDGASolidFillRect
#define DGA_BLIT_RECT		XDGABlitRect
#define DGA_BLIT_RECT_TRANS	XDGABlitTransRect
#define DGA_PIXMAP_AVAILABLE	XDGAPixmap

#define DGA_INTERLACED		XDGAInterlaced
#define DGA_DOUBLESCAN		XDGADoublescan

#define DGA_FLIP_IMMEDIATE	XDGAFlipImmediate
#define DGA_FLIP_RETRACE	XDGAFlipRetrace

#define DGA_NEED_ROOT		XDGANeedRoot

#define DGA_COMPLETED		0x00000000
#define DGA_PENDING		0x00000001

typedef struct {
   int num;		/* A unique identifier for the mode (num > 0) */
   char *name;		/* name of mode given in the XF86Config */
   int VSync_num;
   int VSync_den;
   int flags;		/* DGA_CONCURRENT_ACCESS, etc... */
   int imageWidth;	/* linear accessible portion (pixels) */
   int imageHeight;
   int pixmapWidth;	/* Xlib accessible portion (pixels) */
   int pixmapHeight;	/* both fields ignored if no concurrent access */
   int bytesPerScanline; 
   int byteOrder;	/* MSBFirst, LSBFirst */
   int depth;		
   int bitsPerPixel;
   unsigned long red_mask;
   unsigned long green_mask;
   unsigned long blue_mask;
   short visualClass;
   int viewportWidth;
   int viewportHeight;
   int xViewportStep;	/* viewport position granularity */
   int yViewportStep;
   int maxViewportX;	/* max viewport origin */
   int maxViewportY;
   int viewportFlags;	/* types of page flipping possible */
   int offset;
   int reserved1;
   int reserved2;
} XDGAModeRec, *XDGAModePtr;

/* DDX interface */

int
DGASetMode(
   int Index,
   int num,
   XDGAModePtr mode,
   PixmapPtr *pPix
);

void
DGASetInputMode(
   int Index,
   Bool keyboard,
   Bool mouse
);

void 
DGASelectInput(
   int Index,
   ClientPtr client,
   long mask
);

Bool DGAAvailable(int Index);
Bool DGAActive(int Index);
void DGAShutdown(void);
void DGAInstallCmap(ColormapPtr cmap);
int DGAGetViewportStatus(int Index); 
int DGASync(int Index);

int
DGAFillRect(
   int Index,
   int x, int y, int w, int h,
   unsigned long color
);

int
DGABlitRect(
   int Index,
   int srcx, int srcy, 
   int w, int h, 
   int dstx, int dsty
);

int
DGABlitTransRect(
   int Index,
   int srcx, int srcy, 
   int w, int h, 
   int dstx, int dsty,
   unsigned long color
);

int
DGASetViewport(
   int Index,
   int x, int y,
   int mode
); 

int DGAGetModes(int Index);
int DGAGetOldDGAMode(int Index);

int DGAGetModeInfo(int Index, XDGAModePtr mode, int num);

Bool DGAVTSwitch(void);
Bool DGAStealMouseEvent(int Index, xEvent *e, int dx, int dy);
Bool DGAStealKeyEvent(int Index, xEvent *e);
Bool DGAIsDgaEvent (xEvent *e);

Bool DGADeliverEvent (ScreenPtr pScreen, xEvent *e);
	    
#define NEW_DGAOPENFRAMEBUFFER 1
Bool DGAOpenFramebuffer(int Index, char **name, unsigned int *mem, 
			unsigned int *size, unsigned int *offset,
			unsigned int *flags);
void DGACloseFramebuffer(int Index);
Bool DGAChangePixmapMode(int Index, int *x, int *y, int mode);
int DGACreateColormap(int Index, ClientPtr client, int id, int mode, 
			int alloc);

extern unsigned char DGAReqCode;
extern int DGAErrorBase;
extern int DGAEventBase;
extern int *XDGAEventBase;



#endif /* __DGAPROC_H */
