/*
 *Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 *
 *Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 *"Software"), to deal in the Software without restriction, including
 *without limitation the rights to use, copy, modify, merge, publish,
 *distribute, sublicense, and/or sell copies of the Software, and to
 *permit persons to whom the Software is furnished to do so, subject to
 *the following conditions:
 *
 *The above copyright notice and this permission notice shall be
 *included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of the XFree86 Project
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from the XFree86 Project.
 *
 * Authors:	Harold L Hunt II
 */
/* $XFree86: xc/programs/Xserver/hw/xwin/winwindow.c,v 1.1 2001/04/05 20:13:51 dawes Exp $ */

#include "win.h"

/* See Porting Layer Definition - p. 37 */
/* See mfb/mfbwindow.c - mfbCreateWindow() */
Bool
winCreateWindowNativeGDI (WindowPtr pWin)
{
  fprintf (stderr, "winCreateWindow()\n");
  return TRUE;
}

/* See Porting Layer Definition - p. 37 */
/* See mfb/mfbwindow.c - mfbDestroyWindow() */
Bool
winDestroyWindowNativeGDI (WindowPtr pWin)
{
  fprintf (stderr, "winDestroyWindow()\n");
  return TRUE;
}

/* See Porting Layer Definition - p. 37 */
/* See mfb/mfbwindow.c - mfbPositionWindow() */
Bool
winPositionWindowNativeGDI (WindowPtr pWin, int x, int y)
{
  fprintf (stderr, "winPositionWindow()\n");
  return TRUE;
}

/* See Porting Layer Definition - p. 39 */
/* See mfb/mfbwindow.c - mfbCopyWindow() */
void 
winCopyWindowNativeGDI (WindowPtr pWin,
			DDXPointRec ptOldOrg,
			RegionPtr prgnSrc)
{
  fprintf (stderr, "winCopyWindow()\n");
}

/* See Porting Layer Definition - p. 37 */
/* See mfb/mfbwindow.c - mfbChangeWindowAttributes() */
Bool
winChangeWindowAttributesNativeGDI (WindowPtr pWin, unsigned long mask)
{
  fprintf (stderr, "winChangeWindowAttributes()\n");
  return TRUE;
}

/* See Porting Layer Definition - p. 37
 * Also referred to as UnrealizeWindow
 */
Bool
winUnmapWindowNativeGDI (WindowPtr pWindow)
{
  fprintf (stderr, "winUnmapWindow()\n");
  /* This functions is empty in the CFB,
   * we probably won't need to do anything
   */
  return TRUE;
}

/* See Porting Layer Definition - p. 37
 * Also referred to as RealizeWindow
 */
Bool
winMapWindowNativeGDI (WindowPtr pWindow)
{
  fprintf (stderr, "winMapWindow()\n");
  /* This function is empty in the CFB,
   * we probably won't need to do anything
   */
  return TRUE;
}
