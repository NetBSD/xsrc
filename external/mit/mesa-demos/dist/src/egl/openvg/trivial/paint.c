#include "eglcommon.h"

#include <VG/openvg.h>

#include <stdio.h>
#include <string.h>

static const VGfloat white_color[4] = {1.0, 1.0, 1.0, 1.0};

static VGPath path;
static VGPaint lingrad, radgrad, pattern;

VGColorRampSpreadMode spread = VG_COLOR_RAMP_SPREAD_PAD;

static void
init_linear_gradient(VGPaint paint)
{
   VGfloat rampStop[] = {0.00f, 1.0f, 1.0f, 1.0f, 1.0f,
                         0.33f, 1.0f, 0.0f, 0.0f, 1.0f,
                         0.66f, 0.0f, 1.0f, 0.0f, 1.0f,
                         1.00f, 0.0f, 0.0f,  1.0f, 1.0f};
   VGfloat linearGradient[4] = {25.0f, 25.0f, 75.0f, 75.0f};

   vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
   vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, spread);
   vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, linearGradient);
   vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 20, rampStop);
}

static void
init_radial_gradient(VGPaint paint, VGboolean centered)
{
   VGfloat centeredGradient[5] = {50.0f, 50.0f, 50.0f, 50.0f, 50};
   VGfloat noncenteredGradient[5] = {50.0f, 50.0f, 62.5f, 62.5f, 50};
   VGfloat rampStop[] = {0.00f, 1.0f, 1.0f, 1.0f, 1.0f,
                         0.33f, 1.0f, 0.0f, 0.0f, 1.0f,
                         0.66f, 0.0f, 1.0f, 0.0f, 1.0f,
                         1.00f, 0.0f, 0.0f,  1.0f, 1.0f};
   VGfloat *radialGradient =
      (centered) ? centeredGradient : noncenteredGradient;

   vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
   vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, spread);
   vgSetParameterfv(paint, VG_PAINT_RADIAL_GRADIENT, 5, radialGradient);
   vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 20, rampStop);
}

static void
init_pattern(VGPaint paint)
{
   const VGfloat red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
   const VGfloat green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
   const VGfloat blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
   const VGfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
   VGImage img;

   img = vgCreateImage(VG_sRGBA_8888, 32, 32,
                       VG_IMAGE_QUALITY_NONANTIALIASED);

   vgSetfv(VG_CLEAR_COLOR, 4, red);
   vgClearImage(img, 0, 0, 16, 16);
   vgSetfv(VG_CLEAR_COLOR, 4, green);
   vgClearImage(img, 16, 0, 16, 16);
   vgSetfv(VG_CLEAR_COLOR, 4, blue);
   vgClearImage(img, 0, 16, 16, 16);
   vgSetfv(VG_CLEAR_COLOR, 4, white);
   vgClearImage(img, 16, 16, 16, 16);

   vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
   vgSetParameteri(paint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
   vgPaintPattern(paint, img);
}

static void
init(void)
{
   static const VGubyte sqrCmds[5] = {VG_MOVE_TO_ABS, VG_HLINE_TO_ABS, VG_VLINE_TO_ABS, VG_HLINE_TO_ABS, VG_CLOSE_PATH};
   static const VGfloat sqrCoords[5]   = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f};

   path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0, 0,
                       VG_PATH_CAPABILITY_APPEND_TO);
   vgAppendPathData(path, 5, sqrCmds, sqrCoords);

   lingrad = vgCreatePaint();
   init_linear_gradient(lingrad);

   radgrad = vgCreatePaint();
   init_radial_gradient(radgrad, VG_TRUE);

   pattern = vgCreatePaint();
   init_pattern(pattern);

   vgSetfv(VG_CLEAR_COLOR, 4, white_color);
   vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
}

/* new window size or exposure */
static void
reshape(int w, int h)
{
   vgLoadIdentity();
}

static void
draw_with_paint(VGPaint paint, VGint x, VGint y)
{
   vgSetPaint(paint, VG_FILL_PATH | VG_STROKE_PATH);

   /*
    * The paint and the path are both 100x100.  Scale the paint by 2
    * horizontally and draw the path twice to cover the scaled paint.
    */

   /* scale the paint */
   vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
   vgLoadIdentity();
   vgScale(2.0f, 1);
   vgSeti(VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
   vgLoadIdentity();
   vgScale(2.0f, 1);

   /* draw the left half */
   vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
   vgLoadIdentity();
   vgTranslate(x, y);
   vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);

   /* draw the right half */
   vgTranslate(100, 0);
   vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
   vgTranslate(-50, 0);
   vgSeti(VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
   vgTranslate(-50, 0);

   vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
}

static void
draw(void)
{
   vgClear(0, 0, window_width(), window_height());

   draw_with_paint(lingrad, 100, 25);
   draw_with_paint(radgrad, 100, 150);
   draw_with_paint(pattern, 100, 275);
}


int main(int argc, char **argv)
{
   set_window_size(400, 400);

   return run(argc, argv, init, reshape,
              draw, 0);
}
