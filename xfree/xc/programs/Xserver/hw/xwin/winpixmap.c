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
 * Authors:	drewry, september 1986
 *		Harold L Hunt II
 */
/* $XFree86: xc/programs/Xserver/hw/xwin/winpixmap.c,v 1.3 2001/05/02 00:45:26 alanh Exp $ */

#include "win.h"

/* See Porting Layer Definition - p. 34 */
/* See mfb/mfbpixmap.c - mfbCreatePixmap() */
PixmapPtr
winCreatePixmapNativeGDI (ScreenPtr pScreen,
			  int nWidth, int nHeight,
			  int nDepth)
{
#ifdef CYGX_GDI
  PixmapPtr		pPixmap;
  HBITMAP		hBitmap;
  BITMAPINFOHEADER	bmih;

  fprintf (stderr, "winCreatePixmap()\n");

  /* FIXME: For now we create all pixmaps in system memory.  Pixmaps
     with the same depth as the screen depth can be created in offscreen
     video memory.  It is a simple optimization, but an easy one to
     screw up, so I'm leaving it out in this first implementation
  */

  /* Setup the bitmap header info */
  bmih.biSize = sizeof (bmih);
  bmih.biWidth = nWidth;
  bmih.biHeight = nHeight;
  bmih.biPlanes = 1;
  bmih.biBitCount = nDepth;
  bmih.biCompression = BI_RGB;
  bmih.biSizeImage = 0;
  bmih.biXPelsPerMeter = 0;
  bmih.biYPelsPerMeter = 0;
  bmih.biClrUsed = 0;
  bmih.biClrImportant = 0;

  /* Create the bitmap */
  if (nDepth == 1)
    {
      hBitmap = CreateDIBitmap (NULL, &bmih, 0, NULL, NULL, 0);
    }
  else
    {
      hBitmap = CreateDIBitmap (g_hdcMem, &bmih, 0, NULL, NULL, 0);
    }

  /* Allocate a pixmap structure */
  pPixmap = (PixmapPtr) xalloc (sizeof (PixmapRec));
  if (!pPixmap)
    return NullPixmap;

  /* Set other fields of the pixmap, all fields must be set to
     valid values */
  pPixmap->drawable.type = DRAWABLE_PIXMAP;
  pPixmap->drawable.class = 0;
  pPixmap->drawable.pScreen = pScreen;
  pPixmap->drawable.depth = nDepth;
  pPixmap->drawable.bitsPerPixel = BitsPerPixel (nDepth);
  pPixmap->drawable.id = 0;
  pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
  pPixmap->drawable.x = 0;
  pPixmap->drawable.y = 0;
  pPixmap->drawable.width = nWidth;
  pPixmap->drawable.height = nHeight;
  pPixmap->devKind = nWidth; // Was paddedWidth in mfb
  pPixmap->refcnt = 1;

  /* We will use devPrivate to point to our bitmap */
  pPixmap->devPrivate.ptr = hBitmap;
  
  fprintf (stderr, "winCreatePixmap () - Created a pixmap %08x, %dx%dx%d, for screen: %08x\n",
	   hBitmap, nWidth, nHeight, nDepth, pScreen);

  return pPixmap;
#else /* CYGX_GDI */
  return NULL;
#endif /* CYGX_GDI */
}

/* See Porting Layer Definition - p. 35 */
/* See mfb/mfbpixmap.c - mfbDestroyPixmap() */
Bool
winDestroyPixmapNativeGDI (PixmapPtr pPixmap)
{
  HBITMAP		hBitmap;

  fprintf (stderr, "winDestroyPixmap - pPixmap->devPrivate.ptr: %08x\n",
	   (UINT) pPixmap->devPrivate.ptr);

  /* Decrement reference count, and, if zero, free the pixmap */
  --(pPixmap->refcnt);

  /* Are there any more references to this pixmap? */
  if (pPixmap->refcnt == 0)
    {
      /* Free GDI bitmap */
      hBitmap = pPixmap->devPrivate.ptr;
      if (hBitmap) DeleteObject (hBitmap);
      hBitmap = NULL;

      /* Free the PixmapRec */
      xfree (pPixmap);
      pPixmap = NULL;
    }

  return TRUE;
}

/* See cfb/cfbpixmap.c */
void
winXRotatePixmapNativeGDI (PixmapPtr pPix, int rw)
{
  fprintf (stderr, "winXRotatePixmap()\n");
  /* fill in this function, look at CFB */
}

/* See cfb/cfbpixmap.c */
void
winYRotatePixmapNativeGDI (PixmapPtr pPix, int rh)
{
  fprintf (stderr, "winYRotatePixmap()\n");
  /* fill in this function, look at CFB */
}

/* See cfb/cfbpixmap.c */
void
winCopyRotatePixmapNativeGDI (PixmapPtr psrcPix, PixmapPtr *ppdstPix,
			      int xrot, int yrot)
{
  fprintf (stderr, "winCopyRotatePixmap()\n");
  /* fill in this function, look at CFB */
}
