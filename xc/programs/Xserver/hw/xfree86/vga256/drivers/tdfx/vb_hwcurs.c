/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tdfx/vb_hwcurs.c,v 1.1.2.5 1999/07/13 07:09:54 hohndel Exp $ */
/*
   Voodoo Banshee driver version 1.0.2

   Author: Daryll Strauss

   Copyright: 1998,1999
*/

#include "xf86.h"
#include "xf86_OSlib.h"
#include "xf86cursor.h"
#include "miline.h"

#include "vb.h"

static void VBLoadCursorImage(void *src, int xorig, int yorig);
static void VBShowCursor();
void VBHideCursor();
static void VBSetCursorPosition(int x, int y, int xorig, int yorig);
static void VBSetCursorColors(int bg, int fg);

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();

extern vgaHWCursorRec vgaHWCursor;

Bool
VBCursorInit(char *pm, ScreenPtr pScreen)
{
  VBTRACECURS("VBCursorInit start\n");
  return XAACursorInit(pm, pScreen);
}

Bool
VBHwCursorInit()
{
  VBPtr pVB;
  int i;
  unsigned char *dataptr;

  VBTRACECURS("VBHwCursorInit start\n");
  pVB = VBPTR();
  if (!pVB->CursorData) return FALSE;
  /* Return FALSE if in linedoubled or interleave mode */

  XAACursorInfoRec.MaxWidth = 64;
  XAACursorInfoRec.MaxHeight = 64;
  XAACursorInfoRec.Flags = 
    HARDWARE_CURSOR_BIT_ORDER_MSBFIRST|
    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP|
    HARDWARE_CURSOR_PROGRAMMED_ORIGIN|
    HARDWARE_CURSOR_CHAR_BIT_FORMAT|
    HARDWARE_CURSOR_AND_SOURCE_WITH_MASK|
    HARDWARE_CURSOR_PROGRAMMED_BITS;
  XAACursorInfoRec.SetCursorColors = VBSetCursorColors;
  XAACursorInfoRec.SetCursorPosition = VBSetCursorPosition;
  XAACursorInfoRec.LoadCursorImage = VBLoadCursorImage;
  XAACursorInfoRec.HideCursor = VBHideCursor;
  XAACursorInfoRec.ShowCursor = VBShowCursor;

  vgaHWCursor.Init = VBCursorInit;
  vgaHWCursor.Initialized = TRUE;
  vgaHWCursor.Restore = XAARestoreCursor;
  vgaHWCursor.Warp = XAAWarpCursor;
  vgaHWCursor.QueryBestSize = XAAQueryBestSize;

  return TRUE;
}

static void 
VBLoadCursorImage(void *src, int xorig, int yorig)
{
  int i, j, offset;
  VBPtr pVB;
  unsigned char *dataptr, *srcptr;

  VBTRACECURS("VBLoadCursorImage start src=%x x=%d y=%d\n", src, xorig, yorig);
  pVB = VBPTR();
  dataptr=((unsigned char *)vgaLinearBase)+pVB->CursorData;
  /* Copy the source pixmap in place */
  srcptr=src;
  for (i=0; i<64; i++, dataptr+=16) {
    dataptr[0]=*srcptr++;
    dataptr[8]=*srcptr++;
    dataptr[1]=*srcptr++;
    dataptr[9]=*srcptr++;
    dataptr[2]=*srcptr++;
    dataptr[10]=*srcptr++;
    dataptr[3]=*srcptr++;
    dataptr[11]=*srcptr++;
    dataptr[4]=*srcptr++;
    dataptr[12]=*srcptr++;
    dataptr[5]=*srcptr++;
    dataptr[13]=*srcptr++;
    dataptr[6]=*srcptr++;
    dataptr[14]=*srcptr++;
    dataptr[7]=*srcptr++;
    dataptr[15]=*srcptr++;
  }
}

static void
VBShowCursor()
{
  VBPtr pVB; 
  int vidcfg;

  VBTRACECURS("VBShowCursor start\n");
  pVB = VBPTR();
  vidcfg=inl(pVB->IOBase+VIDPROCCFG);
  vidcfg|=SST_CURSOR_EN;
  outl(pVB->IOBase+VIDPROCCFG, vidcfg);
}

void
VBHideCursor()
{
  VBPtr pVB;
  int vidcfg;

  VBTRACECURS("VBHideCursor start\n");
  pVB = VBPTR();
  vidcfg = inl(pVB->IOBase+VIDPROCCFG);
  vidcfg&=~SST_CURSOR_EN;
  outl(pVB->IOBase+VIDPROCCFG, vidcfg);
}

static void
VBSetCursorPosition(int x, int y, int xorig, int yorig)
{
  VBPtr pVB;

  pVB = VBPTR();
  x-=xorig;
  y-=yorig;
  outl(pVB->IOBase+HWCURLOC, ((y+64)<<16)|(x+64));
}

static void
VBSetCursorColors(int bg, int fg)
{
  VBPtr pVB;

  VBTRACECURS("VBSetCursorColors start\n");
  pVB = VBPTR();
  outl(pVB->IOBase+HWCURC0, bg);
  outl(pVB->IOBase+HWCURC1, fg);
}
