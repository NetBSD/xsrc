/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_vert_form_points.h,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

/* Here is a list of vertex structures that the hardware supports
 * for points.
 */

/* Points with xyzw coords */

/* Non-Textured Points */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
} i830_xyzw_argb_spec_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
} i830_xyzw_argb_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
} i830_xyzw_spec_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
} i830_xyzw_point;

/* Textured Points */

/* Four texunit */

/* Four texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3
} i830_xyzw_argb_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tp0_tp1_tp2_tp3_point;

/* Four texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_tc0_tc1_tc2_tc3_point;

/* Four texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_tp0_tc1_tc2_tc3_point;



/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 cart */


typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_tc0_tp1_tc2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_tc0_tc1_tp2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tc0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq0;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyzw_tp0_tp1_tc2_tc3_point;


/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_tc0_tp1_tp2_tc3_point;


/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tc0_tc1_tp2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tc0_tp1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tp0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_tp0_tc1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_argb_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyzw_tp0_tp1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tp0_tc1_tp2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tp0_tp1_tc2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_argb_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyzw_tc0_tp1_tp2_tp3_point;


/* Three texunit */

/* Three texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_tp0_tp1_tp2_point;

/* Three texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_tc0_tc1_tc2_point;

/* Three texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyzw_tp0_tc1_tc2_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_tp0_tp1_tc2_point;

/* Unit 0 proj : Unit 1 cart: Unit 2 proj */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_tp0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_argb_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyzw_tc0_tp1_tc2_point;

/* Unit 0 cart : Unit 1 cart: Unit 2 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_tc0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 proj */

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_argb_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyzw_tc0_tp1_tp2_point;


/* Two texunit */

/* Two texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_argb_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_argb_tp0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_tp0_tp1_point;

/* Two texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyzw_argb_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyzw_argb_tc0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyzw_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyzw_tc0_tc1_point;

/* Two texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyzw_argb_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyzw_argb_tp0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyzw_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyzw_tp0_tc1_point;

/* Unit 0 cart : Unit 1 proj */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_argb_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_argb_tc0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyzw_tc0_tp1_point;

/* One texunit */

/* One texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyzw_argb_spec_tp0_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyzw_argb_tp0_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyzw_spec_tp0_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   float tq0;
} i830_xyzw_tp0_point;

/* One texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xyzw_argb_spec_tc0_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
} i830_xyzw_argb_tc0_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xyzw_spec_tc0_point;

typedef struct {
   float x;
   float y;
   float z;
   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
} i830_xyzw_tc0_point;


/* Points w/ xyz coords */

/* Non-Textured Points */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
} i830_xyz_argb_spec_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
} i830_xyz_argb_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
} i830_xyz_spec_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
} i830_xyz_point;

/* Textured Points */

/* Four texunit */

/* Four texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3
} i830_xyz_argb_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tp0_tp1_tp2_tp3_point;

/* Four texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_tc0_tc1_tc2_tc3_point;

/* Four texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_tp0_tc1_tc2_tc3_point;



/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 cart */


typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_tc0_tp1_tc2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_tc0_tc1_tp2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tc0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq0;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyz_tp0_tp1_tc2_tc3_point;


/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_tc0_tp1_tp2_tc3_point;


/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tc0_tc1_tp2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tc0_tp1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tp0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_tp0_tc1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_argb_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyz_tp0_tp1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tp0_tc1_tp2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tp0_tp1_tc2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_argb_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyz_tc0_tp1_tp2_tp3_point;


/* Three texunit */

/* Three texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_tp0_tp1_tp2_point;

/* Three texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_tc0_tc1_tc2_point;

/* Three texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyz_tp0_tc1_tc2_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_tp0_tp1_tc2_point;

/* Unit 0 proj : Unit 1 cart: Unit 2 proj */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_tp0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_argb_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyz_tc0_tp1_tc2_point;

/* Unit 0 cart : Unit 1 cart: Unit 2 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_tc0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 proj */

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_argb_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyz_tc0_tp1_tp2_point;


/* Two texunit */

/* Two texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_argb_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_argb_tp0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_tp0_tp1_point;

/* Two texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyz_argb_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyz_argb_tc0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyz_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyz_tc0_tc1_point;

/* Two texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyz_argb_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyz_argb_tp0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyz_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyz_tp0_tc1_point;

/* Unit 0 cart : Unit 1 proj */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_argb_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_argb_tc0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyz_tc0_tp1_point;

/* One texunit */

/* One texunit - all proj */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyz_argb_spec_tp0_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyz_argb_tp0_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyz_spec_tp0_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;

   float tu0;
   float tv0;
   float tq0;
} i830_xyz_tp0_point;

/* One texunit - all cart */
typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xyz_argb_spec_tc0_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
} i830_xyz_argb_tc0_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xyz_spec_tc0_point;

typedef struct {
   float x;
   float y;
   float z;

   float point_width;

   float tu0;
   float tv0;
} i830_xyz_tc0_point;


/* Points w/ xy coords */

/* Non-Textured Points */
typedef struct {
   float x;
   float y;    

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
} i830_xy_argb_spec_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
} i830_xy_argb_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
} i830_xy_spec_point;

typedef struct {
   float x;
   float y;

   float point_width;
} i830_xy_point;

/* Textured Points */

/* Four texunit */

/* Four texunit - all proj */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3
} i830_xy_argb_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tp0_tp1_tp2_tp3_point;

/* Four texunit - all cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_tc0_tc1_tc2_tc3_point;

/* Four texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_tp0_tc1_tc2_tc3_point;



/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 cart */


typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_tc0_tp1_tc2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_tc0_tc1_tp2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tc0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 cart */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq0;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xy_tp0_tp1_tc2_tc3_point;


/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_tc0_tp1_tp2_tc3_point;


/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tc0_tc1_tp2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tc0_tp1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tp0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_tp0_tc1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_argb_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xy_tp0_tp1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tp0_tc1_tp2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tp0_tp1_tc2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_argb_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xy_tc0_tp1_tp2_tp3_point;


/* Three texunit */

/* Three texunit - all proj */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_tp0_tp1_tp2_point;

/* Three texunit - all cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_argb_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_argb_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_tc0_tc1_tc2_point;

/* Three texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_argb_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_argb_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xy_tp0_tc1_tc2_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_argb_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_argb_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_tp0_tp1_tc2_point;

/* Unit 0 proj : Unit 1 cart: Unit 2 proj */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_tp0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_argb_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_argb_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xy_tc0_tp1_tc2_point;

/* Unit 0 cart : Unit 1 cart: Unit 2 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_tc0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 proj */

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_argb_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xy_tc0_tp1_tp2_point;


/* Two texunit */

/* Two texunit - all proj */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_argb_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_argb_tp0_tp1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;

   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_tp0_tp1_point;

/* Two texunit - all cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xy_argb_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xy_argb_tc0_tc1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xy_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;

   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xy_tc0_tc1_point;

/* Two texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xy_argb_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xy_argb_tp0_tc1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xy_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;

   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xy_tp0_tc1_point;

/* Unit 0 cart : Unit 1 proj */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_argb_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_argb_tc0_tp1_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;

   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xy_tc0_tp1_point;

/* One texunit */

/* One texunit - all proj */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xy_argb_spec_tp0_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
} i830_xy_argb_tp0_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xy_spec_tp0_point;

typedef struct {
   float x;
   float y;

   float point_width;

   float tu0;
   float tv0;
   float tq0;
} i830_xy_tp0_point;

/* One texunit - all cart */
typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xy_argb_spec_tc0_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
} i830_xy_argb_tc0_point;

typedef struct {
   float x;
   float y;

   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xy_spec_tc0_point;

typedef struct {
   float x;
   float y;

   float point_width;

   float tu0;
   float tv0;
} i830_xy_tc0_point;

/* Points w/ xyw coords */

/* Non-Textured Points */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
} i830_xyw_argb_spec_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
} i830_xyw_argb_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
} i830_xyw_spec_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
} i830_xyw_point;

/* Textured Points */

/* Four texunit */

/* Four texunit - all proj */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3
} i830_xyw_argb_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tp0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tp0_tp1_tp2_tp3_point;

/* Four texunit - all cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tc0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_tc0_tc1_tc2_tc3_point;

/* Four texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tp0_tc1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_tp0_tc1_tc2_tc3_point;



/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 cart */


typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tc0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_tc0_tp1_tc2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tc0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_tc0_tc1_tp2_tc3_point;

/* Unit 0 cart : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tc0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tc0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 cart */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq0;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tp0_tp1_tc2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
} i830_xyw_tp0_tp1_tc2_tc3_point;


/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tc0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_tc0_tp1_tp2_tc3_point;


/* Unit 0 cart : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tc0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tc0_tc1_tp2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tc0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tc0_tp1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tp0_tc1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tp0_tc1_tc2_tp3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tp0_tc1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_tp0_tc1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 proj : Unit 3 cart */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_spec_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_argb_tp0_tp1_tp2_tc3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
} i830_xyw_tp0_tp1_tp2_tc3_point;

/* Unit 0 proj : Unit 1 cart : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tp0_tc1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tp0_tc1_tp2_tp3_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tp0_tp1_tc2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tp0_tp1_tc2_tp3_point;

/* Unit 0 cart : Unit 1 proj : Unit 2 proj : Unit 3 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_spec_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_argb_tc0_tp1_tp2_tp3_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
   
   float tu3;
   float tv3;
   float tq3;
} i830_xyw_tc0_tp1_tp2_tp3_point;


/* Three texunit */

/* Three texunit - all proj */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_spec_tp0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_tp0_tp1_tp2_point;

/* Three texunit - all cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_spec_tc0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_tc0_tc1_tc2_point;

/* Three texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart : Unit 2 cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_spec_tp0_tc1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
} i830_xyw_tp0_tc1_tc2_point;

/* Unit 0 proj : Unit 1 proj : Unit 2 cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_spec_tp0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_tp0_tp1_tc2_point;

/* Unit 0 proj : Unit 1 cart: Unit 2 proj */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_spec_tp0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_tp0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_argb_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_spec_tc0_tp1_tc2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
} i830_xyw_tc0_tp1_tc2_point;

/* Unit 0 cart : Unit 1 cart: Unit 2 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_spec_tc0_tc1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_tc0_tc1_tp2_point;

/* Unit 0 cart : Unit 1 proj: Unit 2 proj */

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_argb_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_spec_tc0_tp1_tp2_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   
   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
   
   float tu2;
   float tv2;
   float tq2;
} i830_xyw_tc0_tp1_tp2_point;


/* Two texunit */

/* Two texunit - all proj */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_argb_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_argb_tp0_tp1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_spec_tp0_tp1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_tp0_tp1_point;

/* Two texunit - all cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyw_argb_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyw_argb_tc0_tc1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyw_spec_tc0_tc1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
} i830_xyw_tc0_tc1_point;

/* Two texunit mixed cart/proj */

/* Unit 0 proj : Unit 1 cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyw_argb_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyw_argb_tp0_tc1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyw_spec_tp0_tc1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   float tq0;
   
   float tu1;
   float tv1;
} i830_xyw_tp0_tc1_point;

/* Unit 0 cart : Unit 1 proj */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_argb_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_argb_tc0_tp1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_spec_tc0_tp1_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   
   float tu1;
   float tv1;
   float tq1;
} i830_xyw_tc0_tp1_point;

/* One texunit */

/* One texunit - all proj */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyw_argb_spec_tp0_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyw_argb_tp0_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
   float tq0;
} i830_xyw_spec_tp0_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
   float tq0;
} i830_xyw_tp0_point;

/* One texunit - all cart */
typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xyw_argb_spec_tc0_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int argb;

   float tu0;
   float tv0;
} i830_xyw_argb_tc0_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;
   unsigned int fog_spec_rgb;

   float tu0;
   float tv0;
} i830_xyw_spec_tc0_point;

typedef struct {
   float x;
   float y;

   union {
      float w;
      float rhw;
   } w;
   float point_width;

   float tu0;
   float tv0;
} i830_xyw_tc0_point;
