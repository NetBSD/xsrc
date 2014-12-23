#include "eglcommon.h"

#include <VG/openvg.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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

   rect = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                       VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0,
                       VG_PATH_CAPABILITY_ALL);
   vgAppendPathData(rect, sizeof(cmd), cmd, val);

   paint = vgCreatePaint();
   vgSetColor(paint, 0xff0000ff);
   vgSetPaint(paint, VG_FILL_PATH);

   vgSeti(VG_MASKING, VG_TRUE);
}

static void ellipse(VGPath vgPath, VGfloat rx, VGfloat ry, VGfloat angle)
{
    static const VGubyte cmd[] =
    { VG_MOVE_TO_ABS, VG_SCCWARC_TO_REL, VG_SCCWARC_TO_REL, VG_CLOSE_PATH };

    VGfloat val[12];
    VGfloat c = cos(angle) * rx;
    VGfloat s = sin(angle) * rx;

    val[0] = c;
    val[1] = s;
    val[2] = rx;
    val[3] = ry;
    val[4] = angle;
    val[5] = -2.0f * c;
    val[6] = -2.0f * s;
    val[7] = rx;
    val[8] = ry;
    val[9] = angle;
    val[10] = 2.0f * c;
    val[11] = 2.0f * s;

    vgClearPath(vgPath, VG_PATH_CAPABILITY_ALL);
    vgAppendPathData(vgPath, sizeof(cmd), cmd, val);
}

/* new window size or exposure */
static void
reshape(int w, int h)
{
   VGMaskLayer layer;
   VGPath path;
   int i;

   /* test vgFillMaskLayer */
   layer = vgCreateMaskLayer(w, h);
   vgFillMaskLayer(layer, 0, 0, w, h, 0.8f);
   vgMask(layer, VG_SET_MASK, 0, 0, w, h);
   vgDestroyMaskLayer(layer);

   /* test vgRenderToMask */
   path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                       VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0,
                       VG_PATH_CAPABILITY_ALL);
   vgLoadIdentity();
   vgTranslate(w / 2.0f, h / 3.0f);
   ellipse(path, w / 3.0f, h / 3.0f, 0.0f);

   vgRenderToMask(path, VG_FILL_PATH, VG_UNION_MASK);

   vgDestroyPath(path);
}

static const VGfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

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
   vgSetfv(VG_CLEAR_COLOR, 4, white);
   vgClear(0, 0, window_width(), window_height());

   if (window_width() > 10 && window_height() > 10)
      rectangle(5, 5, window_width() - 10, window_height() - 10);
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
