#include "eglcommon.h"

#include <VG/openvg.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

VGboolean verbose;
VGPath rect;

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
   /* alpha = 0.8 */
   vgSetColor(paint, 0xff0000cc);
   vgSetPaint(paint, VG_FILL_PATH);

   vgSeti(VG_MASKING, VG_TRUE);
}

/* new window size or exposure */
static void
reshape(int w, int h)
{
   VGfloat coverage[4] = { 0.0f, 0.0f, 0.0f, 0.4f };
   VGImage img;

   img = vgCreateImage(VG_A_8, w, h / 2, VG_IMAGE_QUALITY_NONANTIALIASED);
   vgSetfv(VG_CLEAR_COLOR, 4, coverage);
   vgClearImage(img, 0, 0, w, h / 2);

   vgMask(img, VG_SET_MASK, 0, (h + 1) / 2, w, h / 2);
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
check_pixel(VGint x, VGint y, VGfloat ref[4], const char *msg)
{
   VGfloat rgba[4];
   VGint pixel;
   VGint i;

   vgReadPixels(&pixel, 0, VG_sRGBA_8888, x, y, 1, 1);
   rgba[0] = ((pixel >> 24) & 0xff) / 255.0f;
   rgba[1] = ((pixel >> 16) & 0xff) / 255.0f;
   rgba[2] = ((pixel >>  8) & 0xff) / 255.0f;
   rgba[3] = ((pixel >>  0) & 0xff) / 255.0f;

   for (i = 0; i < 4; i++) {
      if (fabs(ref[i] - rgba[i]) > 0.1f) {
         printf("Test %s at (%d, %d): expect (%f, %f, %f, %f), got (%f, %f, %f, %f)\n",
               msg, x, y, ref[0], ref[1], ref[2], ref[3],
               rgba[0], rgba[1], rgba[2], rgba[3]);
         break;
      }
   }
}

static void
test_blend(VGint x, VGint y, VGint width, VGint height, VGboolean check)
{
   VGfloat src[4] = { 1.0f, 0.0f, 0.0f, 0.8f};
   VGfloat dst[4] = { 0.0f, 1.0f, 0.0f, 0.3f};
   VGfloat out[4];
   VGint i;

   /* premultiply */
   if (check) {
      for (i = 0; i < 3; i++) {
         src[i] *= src[3];
         dst[i] *= dst[3];
      }

      /*
       * OpenVG allows an implementation to perform interpolation and blending
       * in linear or non-linear color space.  Other factors might also affect
       * the rendering result.  The checks are for reference only.
       */
      printf("Render and check blending result (for reference only)\n");
   }

#define CHECK(msg)            \
   do {                       \
      /* non-premultiply */   \
      for (i = 0; i < 3; i++) \
         out[i] /= out[3];    \
      check_pixel(x + width / 2, y + height / 2, out, msg); \
   } while (0)

   /* 0.8 * red */
   vgSeti(VG_BLEND_MODE, VG_BLEND_SRC);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++)
         out[i] = src[i];
      CHECK("VG_BLEND_SRC");
   }
   x += width + 5;

   /* 0.8 * red + 0.06 * green */
   vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++)
         out[i] = src[i] + dst[i] * (1.0f - src[3]);
      CHECK("VG_BLEND_SRC_OVER");
   }
   x += width + 5;

   /* 0.56 * red + 0.3 * green */
   vgSeti(VG_BLEND_MODE, VG_BLEND_DST_OVER);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++)
         out[i] = dst[i] + src[i] * (1.0f - dst[3]);
      CHECK("VG_BLEND_DST_OVER");
   }
   x += width + 5;

   /* 0.24 * red */
   vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_IN);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++)
         out[i] = src[i] * dst[3];
      CHECK("VG_BLEND_SRC_IN");
   }
   x += width + 5;

   /* 0.24 * green */
   vgSeti(VG_BLEND_MODE, VG_BLEND_DST_IN);
   if (check) {
      for (i = 0; i < 4; i++)
         out[i] = dst[i] * src[3];
      CHECK("VG_BLEND_DST_IN");
   }
   rectangle(x, y, width, height);
   x += width + 5;

   /* (...) * 0.8 * red + 0.06 * green */
   vgSeti(VG_BLEND_MODE, VG_BLEND_MULTIPLY);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 3; i++) {
         out[i] = (1.0f - dst[3]) * src[i] +
            (1.0f - src[3]) * dst[i] + src[i] * dst[i];
      }
      out[3] = src[3] + dst[3] * (1.0f - src[3]);
      CHECK("VG_BLEND_MULTIPLY");
   }
   x += width + 5;

   /* 0.8 * red + (white - 0.8 * red) * 0.3 * green */
   vgSeti(VG_BLEND_MODE, VG_BLEND_SCREEN);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++)
         out[i] = src[i] + (1.0f - src[i]) * dst[i];
      CHECK("VG_BLEND_SCREEN");
   }
   x += width + 5;

   /* min(SRC_OVER, DST_OVER) */
   vgSeti(VG_BLEND_MODE, VG_BLEND_DARKEN);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++) {
         VGfloat src_over = src[i] + dst[i] * (1.0f - src[3]);
         VGfloat dst_over = dst[i] + src[i] * (1.0f - dst[3]);
         out[i] = (src_over < dst_over) ? src_over : dst_over;
      }
      CHECK("VG_BLEND_DARKEN");
   }
   x += width + 5;

   /* max(SRC_OVER, DST_OVER) */
   vgSeti(VG_BLEND_MODE, VG_BLEND_LIGHTEN);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++) {
         VGfloat src_over = src[i] + dst[i] * (1.0f - src[3]);
         VGfloat dst_over = dst[i] + src[i] * (1.0f - dst[3]);
         out[i] = (src_over > dst_over) ? src_over : dst_over;
      }
      CHECK("VG_BLEND_LIGHTEN");
   }
   x += width + 5;

   vgSeti(VG_BLEND_MODE, VG_BLEND_ADDITIVE);
   rectangle(x, y, width, height);
   if (check) {
      for (i = 0; i < 4; i++) {
         out[i] = src[i] + dst[i];
         if (out[i] > 1.0f)
            out[i] = 1.0f;
      }
      CHECK("VG_BLEND_ADDITIVE");
   }
   x += width + 5;

   if (check)
      printf("done\n");
}

static void
draw(void)
{
   const VGfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
   const VGfloat green[4] = { 0.0f, 1.0f, 0.0f, 0.3f };
   VGint x, y;

   vgSetfv(VG_CLEAR_COLOR, 4, white);
   vgClear(0, 0, window_width(), window_height());

   vgSetfv(VG_CLEAR_COLOR, 4, green);

   x = y = 5;
   vgClear(x, y, window_width(), 20);
   test_blend(x, y, 20, 20, verbose);

   y += window_height() / 2;
   vgClear(x, y, window_width(), 20);
   /*
    * This will have more green because
    *
    *   result = blended * coverage + dst * (1 - coverage)
    *
    * Note that impplementations may choose a different formula.
    */
   test_blend(x, y, 20, 20, VG_FALSE);
}


int main(int argc, char **argv)
{
   if (argc > 1) {
      const char *arg = argv[1];
      if (!strcmp("-v", arg))
         verbose = VG_TRUE;
   }

   set_window_size(300, 300);
   set_window_alpha_size(1);
   return run(argc, argv, init, reshape,
              draw, 0);
}
