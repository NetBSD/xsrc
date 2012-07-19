/*
 * DRI wrapper for 300 and 315 series
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria
 *
 * Preliminary 315/330 support by Thomas Winischhofer
 * Portions of Mesa 4/5 changes by Eric Anholt
 *
 * Licensed under the following terms:
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appears in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * and that the name of the copyright holder not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. The copyright holder makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without expressed or implied warranty.
 *
 * THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Previously taken and modified from tdfx_dri.c, mga_dri.c
 *
 * Authors:	Can-Ru Yeou, SiS Inc.
 *		Alan Hourihane, Wigan, England,
 *		Thomas Winischhofer <thomas@winischhofer.net>
 *		others.
 */

#ifndef _XGI_DRI_
#define _XGI_DRI_

#include <xf86drm.h>

#define XGI_MAX_DRAWABLES 256
#define XGIIOMAPSIZE (64*1024)

typedef struct {
  int CtxOwner;
  int QueueLength;
  unsigned long AGPVtxBufNext;
  unsigned int FrameCount;
  
  unsigned long shareWPoffset;
  /*CARD16*/
  /*unsigned short RelIO;*/

  /* 2001/12/16 added by jjtseng for some bala reasons .... */
  unsigned char *AGPCmdBufBase;
  unsigned long AGPCmdBufAddr;
  unsigned long AGPCmdBufOffset;
  unsigned int  AGPCmdBufSize;
  unsigned long AGPCmdBufNext;
  /*~ 2001/12/16 added by jjtseng for some bala reasons .... */
  /* chiawen@2005/0601 for agp heap */  
  int isAGPHeapCreated;
} XGISAREAPriv;

#define XGI_FRONT 0
#define XGI_BACK 1
#define XGI_DEPTH 2

typedef struct {
  drm_handle_t handle;
  drmSize size;
  drmAddress map;
} xgiRegion, *xgiRegionPtr;

typedef struct {
  xgiRegion regs, agp;
  int deviceID;
  int revisionID;
  int width;
  int height;
  int mem;
  int bytesPerPixel;
  int priv1;
  int priv2;
  int fbOffset;
  int backOffset;
  int depthOffset;
  int textureOffset;
  int textureSize;
  unsigned int AGPVtxBufOffset;
  unsigned int AGPVtxBufSize;
  /* 2001/12/16 added by jjtseng for some bala reasons .... */
  unsigned char *AGPCmdBufBase;
  unsigned long AGPCmdBufAddr;
  unsigned long AGPCmdBufOffset;
  unsigned int AGPCmdBufSize;
  unsigned long *pAGPCmdBufNext;
  /*~ 2001/12/16 added by jjtseng for some bala reasons .... */
  int irqEnabled;
  unsigned int scrnX, scrnY;
} XGIDRIRec, *XGIDRIPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} XGIConfigPrivRec, *XGIConfigPrivPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} XGIDRIContextRec, *XGIDRIContextPtr;

Bool XGIDRIScreenInit(ScreenPtr pScreen);
void XGIDRICloseScreen(ScreenPtr pScreen);
Bool XGIDRIFinishScreenInit(ScreenPtr pScreen);

#endif
