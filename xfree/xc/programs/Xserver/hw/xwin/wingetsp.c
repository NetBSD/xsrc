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
/* $XFree86: xc/programs/Xserver/hw/xwin/wingetsp.c,v 1.1 2001/04/05 20:13:49 dawes Exp $ */

#include "win.h"

/* See Porting Layer Definition - p. 55 */
void
winGetSpansNativeGDI (DrawablePtr	pDrawable, 
		      int		nMax, 
		      DDXPointPtr	pPoints, 
		      int		*pWidths, 
		      int		nSpans, 
		      char		*pDsts)
{
#if 0
  int			iIdx;
  DDXPointPtr		pPoint = NULL;
  int			*pWidth = NULL;
  char			*pDst = pDsts;
  int			iScanlineBytes;
  int			iBitmapBytes;
  BITMAPINFOHEADER	bmih;
  DEBUG_FN_NAME("winGetSpans");
  DEBUGVARS;
  DEBUGPROC_MSG;

  /* Setup the bitmap header info */
  bmih.biSize = sizeof (bmih);
  bmih.biWidth = pDrawable->width;
  bmih.biHeight = pDrawable->height;
  bmih.biPlanes = 1;
  bmih.biBitCount = pDrawable->depth;
  bmih.biCompression = BI_RGB;
  bmih.biSizeImage = 0;
  bmih.biXPelsPerMeter = 0;
  bmih.biYPelsPerMeter = 0;
  bmih.biClrUsed = 0;
  bmih.biClrImportant = 0;

  fprintf (stderr, "winGetSpans () - pDrawable: %08x\n",
	   pDrawable);

  /* Calculate the number of bytes in each scanline */
  iScanlineBytes = 4 * ((pDrawable->width * pDrawable->depth + 31) / 32);
  
  /* Calculate the number of bytes in the bitmap */
  iBitmapBytes = iScanlineBytes * pDrawable->height;

  /*
    fprintf (stderr, "winGetSpans () - iBitmapBytes: %d\n",
    iBitmapBytes);
  */
  
  /* Loop through spans */
  for (iIdx = 0; iIdx < nSpans; ++iIdx)
    {
      pPoint = pPoints + iIdx;
      pWidth = pWidths + iIdx;

      /* Drawable should be in g_hdcMem */
      GetDIBits (hdcMem, ((PixmapPtr)pDrawable)->devPrivate.ptr,
		 pPoint->y, 1, pDst, &bmih, 0);

      fprintf (stderr, "(%dx%dx%d) (%d,%d) w: %d\n",
	       pDrawable->width, pDrawable->height, pDrawable->depth,
	       pPoint->x, pPoint->y, *pWidth);

      /* Calculate offset of next bit destination */
      pDst += 4 * ((*pWidth + 31) / 32);
    }
#endif
}
