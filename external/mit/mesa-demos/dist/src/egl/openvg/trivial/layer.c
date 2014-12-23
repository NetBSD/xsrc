#include "eglcommon.h"

#include <VG/openvg.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef OPENVG_VERSION_1_1

#define NUM_LAYERS 2

static VGMaskLayer layers[NUM_LAYERS];
static VGint current_layer;
static VGPath path;

static void
init(void)
{
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
   int i;

   if (path)
      vgDestroyPath(path);
   path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                       VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0,
                       VG_PATH_CAPABILITY_ALL);

   for (i = 0; i < NUM_LAYERS; i++) {
      if (layers[i])
         vgDestroyMaskLayer(layers[i]);
      layers[i] = vgCreateMaskLayer(w, h);
   }

   vgLoadIdentity();
   vgTranslate(w / 2.0f, h / 2.0f);
   ellipse(path, w / 3.0f, h / 3.0f, 0.0f);

   /* test vgFillMaskLayer and vgCopyMask */
   vgFillMaskLayer(layers[0], 0, 0, w, h / 2, 0.8f);
   vgFillMaskLayer(layers[0], 0, h / 2, w, h / 2, 0.4f);
   vgMask(layers[0], VG_SET_MASK, 0, 0, w, h);
   vgCopyMask(layers[1], 0, 0, 0, 0, w, h);
}

static const VGfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static void
draw(void)
{
   vgSetfv(VG_CLEAR_COLOR, 4, white);
   vgClear(0, 0, window_width(), window_height());

   vgDrawPath(path, VG_FILL_PATH);

   current_layer = (current_layer + 1) % NUM_LAYERS;
   vgMask(layers[current_layer], VG_SET_MASK,
         0, 0, window_width(), window_height());
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
