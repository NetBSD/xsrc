#include "eglcommon.h"

#include <VG/openvg.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static void
init(void)
{
}

/* new window size or exposure */
static void
reshape(int w, int h)
{
}

static const VGfloat red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
static const VGfloat black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
static const VGfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static void
draw(void)
{
   const VGint w = 48;
   VGImage img1, img2;
   VGint x, y;

   vgSetfv(VG_CLEAR_COLOR, 4, white);
   vgClear(0, 0, window_width(), window_height());

   img1 = vgCreateImage(VG_sRGBA_8888, w, w,
         VG_IMAGE_QUALITY_NONANTIALIASED);
   img2 = vgCreateImage(VG_sRGBA_8888, w, w,
         VG_IMAGE_QUALITY_NONANTIALIASED);

   x = 5;
   y = (window_height() - w) / 2;

   /* test vgSetPixels */
   vgSetfv(VG_CLEAR_COLOR, 4, red);
   vgClearImage(img1, 0, 0, w, w / 2);
   vgSetfv(VG_CLEAR_COLOR, 4, black);
   vgClearImage(img1, 0, w / 2, w, w / 2);
   vgSetPixels(x, y, img1, 0, 0, w, w);

   x += w + 5;

   /* test vgDrawImage */
   vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
   vgLoadIdentity();
   vgTranslate(x, y);
   vgDrawImage(img1);

   /* test vgGetPixels */
   vgGetPixels(img1, 0, 0, x, y, w, w);
   x += w + 5;
   vgSetPixels(x, y, img1, 0, 0, w, w);

   x += w + 5;

   /* test vgCopyImage */
   vgCopyImage(img2, 0, 0, img1, 0, 0, w, w, VG_FALSE);
   vgSetPixels(x, y, img2, 0, 0, w, w);

   /* vgCopyPixels */
   vgCopyPixels(x + w + 5, y, x, y, w, w);

   vgDestroyImage(img1);
   vgDestroyImage(img2);
}


int main(int argc, char **argv)
{
   set_window_size(300, 300);
   return run(argc, argv, init, reshape,
              draw, 0);
}
