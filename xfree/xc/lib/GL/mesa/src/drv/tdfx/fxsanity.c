/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxsanity.c,v 1.2 2000/12/08 19:36:23 alanh Exp $ */
/*
 * Mesa 3-D graphics library
 * Version:  3.3
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Original Mesa / 3Dfx device driver (C) 1999 David Bucciarelli, by the
 * terms stated above.
 *
 * Thank you for your contribution, David!
 *
 * Please make note of the above copyright/license statement.  If you
 * contributed code or bug fixes to this code under the previous (GNU
 * Library) license and object to the new license, your code will be
 * removed at your request.  Please see the Mesa docs/COPYRIGHT file
 * for more information.
 *
 * Additional Mesa/3Dfx driver developers:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *   Keith Whitwell <keith@precisioninsight.com>
 *
 * See fxapi.h for more revision/author details.
 */


#include "fxdrv.h"

/* I have found this quite useful in tracking down transformation &
 * clipping bugs.  If you get a random graphics card freeze, running
 * your triangles through this will probably catch the problem.
 */

#define WID 640
#define HI  480

#undef grDrawTriangle

void
fx_sanity_triangle(fxMesaContext fxMesa,
                   GrVertex * v1, GrVertex * v2, GrVertex * v3)
{
    GLuint rv = 1, print = 0;

    GLfloat area = ((v1->x - v3->x) * (v2->y - v3->y) -
                    (v1->y - v3->y) * (v2->x - v3->x));

    if (v1->x < 0 || v1->y < 0 || v1->x > WID || v1->y > HI ||
        v2->x < 0 || v2->y < 0 || v2->x > WID || v2->y > HI ||
        v3->x < 0 || v3->y < 0 || v3->x > WID || v3->y > HI) {
        fprintf(stderr, "not clipped/set up!!!!!\n");
        rv = 0;
        print = 1;
    }

    if (area > (WID * HI)) {
        fprintf(stderr, "too big\n");
        rv = 0;
    }
    if (v1->oow == 0 || v2->oow == 0 || v3->oow == 0) {
        fprintf(stderr, "zero oow\n");
        rv = 0;
    }
    if (0 && area == 0) {
        fprintf(stderr, "zero area %p %p %p\n", v1, v2, v3);
        rv = 0;
    }

    if (print) {
#if FX_USE_PARGB
        fprintf(stderr,
                "v1: %f %f %f %f col %d %d %d %d t0 %f %f %f t1 %f %f %f\n",
                v3->x, v3->y, v3->ooz, v3->oow, GET_PR(v1), GET_PG(v1), GET_PB(v1), GET_PA(v1),
                v1->tmuvtx[0].sow, v1->tmuvtx[0].tow, v1->tmuvtx[0].oow,
                v1->tmuvtx[1].sow, v1->tmuvtx[1].tow, v1->tmuvtx[1].oow);
        fprintf(stderr,
                "v2: %f %f %f %f col %d %d %d %d t0 %f %f %f t1 %f %f %f\n",
                v3->x, v3->y, v3->ooz, v3->oow, GET_PR(v1), GET_PG(v1), GET_PB(v1), GET_PA(v1),
                v2->tmuvtx[0].sow, v2->tmuvtx[0].tow, v2->tmuvtx[0].oow,
                v2->tmuvtx[1].sow, v2->tmuvtx[1].tow, v2->tmuvtx[1].oow);
        fprintf(stderr,
                "v3: %f %f %f %f col %d %d %d %d t0 %f %f %f t1 %f %f %f\n",
                v3->x, v3->y, v3->ooz, v3->oow, GET_PR(v1), GET_PG(v1), GET_PB(v1), GET_PA(v1),
                v3->tmuvtx[0].sow, v3->tmuvtx[0].tow, v3->tmuvtx[0].oow,
                v3->tmuvtx[1].sow, v3->tmuvtx[1].tow, v3->tmuvtx[1].oow);
#else
        fprintf(stderr,
                "v1: %f %f %f %f col %.0f %.0f %.0f %.0f t0 %f %f %f t1 %f %f %f\n",
                v1->x, v1->y, v1->ooz, v1->oow, v1->r, v1->g, v1->b, v1->a,
                v1->tmuvtx[0].sow, v1->tmuvtx[0].tow, v1->tmuvtx[0].oow,
                v1->tmuvtx[1].sow, v1->tmuvtx[1].tow, v1->tmuvtx[1].oow);
        fprintf(stderr,
                "v2: %f %f %f %f col %.0f %.0f %.0f %.0f t0 %f %f %f t1 %f %f %f\n",
                v2->x, v2->y, v2->ooz, v2->oow, v2->r, v2->g, v2->b, v2->a,
                v2->tmuvtx[0].sow, v2->tmuvtx[0].tow, v2->tmuvtx[0].oow,
                v2->tmuvtx[1].sow, v2->tmuvtx[1].tow, v2->tmuvtx[1].oow);
        fprintf(stderr,
                "v3: %f %f %f %f col %.0f %.0f %.0f %.0f t0 %f %f %f t1 %f %f %f\n",
                v3->x, v3->y, v3->ooz, v3->oow, v3->r, v3->g, v3->b, v3->a,
                v3->tmuvtx[0].sow, v3->tmuvtx[0].tow, v3->tmuvtx[0].oow,
                v3->tmuvtx[1].sow, v3->tmuvtx[1].tow, v3->tmuvtx[1].oow);
#endif
    }

    if (1)
        FX_grDrawTriangle(fxMesa, v1, v2, v3);
    else
        fprintf(stderr, "\n\n\n");
}

