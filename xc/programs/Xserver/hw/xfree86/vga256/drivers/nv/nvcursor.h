/* $XConsortium: nvreg.h /main/2 1996/10/28 05:13:41 kaleb $ */
/*
 * Copyright 1996-1997  David J. McKay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nvcursor.h,v 1.1.2.2 1998/01/24 00:04:40 robin Exp $ */

#ifndef _NVCURSOR_H_
#define _NVCURSOR_H_

Bool NVCursorInit(char *pm,ScreenPtr pScr);
void NVRestoreCursor(ScreenPtr pScr);
void NVWarpCursor(ScreenPtr pScr,int x,int y);
void NVQueryBestSize(int class,unsigned short *pwidth, 
                     unsigned short *pheight,ScreenPtr pScreen);


Bool NV3CursorInit(char *pm,ScreenPtr pScr);
void NV3RestoreCursor(ScreenPtr pScr);
void NV3WarpCursor(ScreenPtr pScr,int x,int y);
void NV3QueryBestSize(int class,unsigned short *pwidth, 
                     unsigned short *pheight,ScreenPtr pScreen);

#endif
