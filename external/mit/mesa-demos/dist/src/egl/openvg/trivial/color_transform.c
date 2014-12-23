#include "eglcommon.h"

#include <VG/openvg.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef OPENVG_VERSION_1_1

static VGPath rect;

static void
init(void)
{
   VGPaint paint;

   VGubyte cmd[] = {
      VG_MOVE_TO_ABS,
      VG_LINE_TO_ABS,
      VG_LINE_TO_ABS,
      VG_LINE_TO_ABS,
      VG_CLOSE_PATH
   };
   VGfloat val[] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      1.0f, 1.0f,
      0.0f, 1.0f
   };
   const VGfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };


   rect = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                       VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0,
                       VG_PATH_CAPABILITY_ALL);
   vgAppendPathData(rect, sizeof(cmd), cmd, val);

   paint = vgCreatePaint();
   vgSetColor(paint, 0xffffffff);
   vgSetPaint(paint, VG_FILL_PATH);

   vgSetfv(VG_CLEAR_COLOR, 4, white);

   vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
   vgSeti(VG_MASKING, VG_TRUE);
}

/* new window size or exposure */
static void
reshape(int w, int h)
{
   /* test color transform and masking interaction */
   vgMask(VG_INVALID_HANDLE, VG_FILL_MASK, 0, 0, w, h);
   vgMask(VG_INVALID_HANDLE, VG_CLEAR_MASK, 0, 10, w, 2);
   vgMask(VG_INVALID_HANDLE, VG_CLEAR_MASK, 10, 0, 2, h);
}

static void
rectangle(VGint x, VGint y, VGint width, VGint height)
{
   vgLoadIdentity();
   vgTranslate(x, y);
   vgScale(width, height);
   vgDrawPath(rect, VG_FILL_PATH);
}

static void
draw(void)
{
   VGfloat xform[8], identity[8] = {
      1.0f, 1.0f, 1.0f, 1.0f,
      0.0f, 0.0f, 0.0f, 0.0f
   };
   VGint w = window_width();
   VGint h = window_height();

   vgClear(0, 0, w, h);

   w /= 2;
   h /= 2;

   /* bottom left: red */
   memcpy(xform, identity, sizeof(identity));
   xform[0] = 0.5f;
   xform[1] = 0.0f;
   xform[2] = 0.0f;
   xform[4] = 0.5f;
   vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, xform);
   rectangle(0, 0, w, h);

   /* bottom right: green */
   memcpy(xform, identity, sizeof(identity));
   xform[0] = 0.0f;
   xform[1] = 2.0f;
   xform[2] = 0.0f;
   xform[5] = -1.0f;
   vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, xform);
   rectangle(w, 0, w, h);

   /* top left: blue */
   memcpy(xform, identity, sizeof(identity));
   xform[0] = 0.0f;
   xform[1] = 0.0f;
   xform[2] = 200.0f;
   xform[6] = -200.0f;
   vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, xform);
   rectangle(0, h, w, h);

   /* top right: black */
   memcpy(xform, identity, sizeof(identity));
   xform[0] = -1.0f;
   xform[1] = -200.0f;
   xform[2] = 0.0f;
   xform[4] = 1.0f;
   vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, xform);
   rectangle(w, h, w, h);
}

int main(int argc, char **argv)
{
   set_window_size(300, 300);
   return run(argc, argv, init, reshape,
              draw, 0);
}

#else /* OPENVG_VERSION_1_1 */

int main(int argc, char **argv)
{
   printf("This demo requires OpenVG 1.1\n");
   return 0;
}

#endif /* OPENVG_VERSION_1_1 */
