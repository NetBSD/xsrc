/*
 * GLX Hardware Device Driver for Matrox Millenium G200
 * Copyright (C) 1999 Wittawat Yamwong
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
 * WITTAWAT YAMWONG, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    Wittawat Yamwong <Wittawat.Yamwong@stud.uni-hannover.de>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgatris.h,v 1.5 2000/09/24 13:51:08 alanh Exp $ */

#ifndef MGATIS_INC
#define MGATIS_INC

#include "types.h"
#include "mgaioctl.h"

extern void mgaDDChooseRenderState(GLcontext *ctx);
extern void mgaDDTrifuncInit( void );


#define MGA_FLAT_BIT	    0x1
#define MGA_OFFSET_BIT	    0x2	
#define MGA_TWOSIDE_BIT	    0x4	
#define MGA_NODRAW_BIT	    0x8
#define MGA_FALLBACK_BIT    0x10

extern int nrswaps;


#define VERTSIZE vertsize

/*  static float _v0[] = { */
/*     242.202515, */
/*     250.469604, */
/*     0.961081, */
/*     0.013503, */
/*     0, */
/*     0.000000, */
/*     2.600000, */
/*     0.000000, */
/*     1.000000, */
/*     0.600000 */
/*  }; */

/*  static float _v1[] = { */
/*     246.448914, */
/*     54.697876, */
/*     0.953817, */
/*     0.014156, */
/*     0, */
/*     0.000000, */
/*     2.600000, */
/*     2.000000, */
/*     1.000000, */
/*     1.600000 */
/*  }; */

/*  static float _v2[] = { */
/*     55.999474, */
/*     85.196106, */
/*     0.942609, */
/*     0.015165, */
/*     0, */
/*     0.000000, */
/*     0.600000, */
/*     2.000000, */
/*     0.000000, */
/*     1.600000, */
/*  }; */

static __inline void mga_draw_triangle( mgaContextPtr mmesa,
				      mgaVertex *v0, 
				      mgaVertex *v1, 
				      mgaVertex *v2 )
{
   GLuint vertsize = mmesa->vertsize;
   GLuint *wv = mgaAllocVertexDwordsInline( mmesa, 3 * VERTSIZE );
   int j;

   (void) vertsize;

/*     for (j = 0 ; j < vertsize ; j++)  */
/*        fprintf(stderr, "v0 %d: %f 0x%x\n", j, v0->f[j], v0->ui[j]); */

/*     for (j = 0 ; j < vertsize ; j++)  */
/*        fprintf(stderr, "v1 %d: %f 0x%x\n", j, v1->f[j], v1->ui[j]); */

/*     for (j = 0 ; j < vertsize ; j++)  */
/*        fprintf(stderr, "v2 %d: %f 0x%x\n", j, v2->f[j], v2->ui[j]); */

/*     v0 = (mgaVertex *)_v0; */
/*     v1 = (mgaVertex *)_v1; */
/*     v2 = (mgaVertex *)_v2; */


/*     if (v0->v.x < 0 || v0->v.x > 1920 || */
/*         v0->v.y < 0 || v0->v.y > 1440 || */
/*         v0->v.z < 0 || v0->v.z > 1)  */
/*        fprintf(stderr, "v0 %f %f %f %f\n", v0->v.x, v0->v.y, v0->v.z, v0->v.rhw); */


/*     if (v1->v.x < 0 || v1->v.x > 1920 || */
/*         v1->v.y < 0 || v1->v.y > 1440 || */
/*         v1->v.z < 0 || v1->v.z > 1)  */
/*        fprintf(stderr, "v1 %f %f %f %f\n", v1->v.x, v1->v.y, v1->v.z, v1->v.rhw); */

/*     if (v2->v.x < 0 || v2->v.x > 1920 || */
/*         v2->v.y < 0 || v2->v.y > 1440 || */
/*         v2->v.z < 0 || v2->v.z > 1)  */
/*        fprintf(stderr, "v2 %f %f %f %f\n", v2->v.x, v2->v.y, v2->v.z, v2->v.rhw); */
      

#if defined (USE_X86_ASM)
   /* GTH: We can safely assume the vertex stride is some number of
    * dwords, and thus a "rep movsd" is okay.  The vb pointer is
    * automagically updated with this instruction, so we don't have
    * to manually take care of incrementing it.
    */
      __asm__ __volatile__( "rep ; movsl"
			    : "=%c" (j)
			    : "0" (VERTSIZE), "D" ((long)wv), "S" ((long)v0)
			    : "memory" );
      __asm__ __volatile__( "rep ; movsl"
			    : "=%c" (j)
			    : "0" (VERTSIZE), "S" ((long)v1)
			    : "memory" );
      __asm__ __volatile__( "rep ; movsl"
			    : "=%c" (j)
			    : "0" (VERTSIZE), "S" ((long)v2)
			    : "memory" );
#else
   {
      for (j = 0 ; j < vertsize ; j++) 
	 wv[j] = v0->ui[j];

      wv += VERTSIZE;
      for (j = 0 ; j < vertsize ; j++) 
	 wv[j] = v1->ui[j];

      wv += VERTSIZE;
      for (j = 0 ; j < vertsize ; j++) 
	 wv[j] = v2->ui[j];
   }
#endif
}


static __inline void mga_draw_point( mgaContextPtr mmesa,
				   mgaVertex *tmp, float sz )
{
   GLuint vertsize = mmesa->vertsize;
   GLuint *wv = mgaAllocVertexDwords( mmesa, 6*VERTSIZE);
   int j;

   *(float *)&wv[0] = tmp->v.x - sz;
   *(float *)&wv[1] = tmp->v.y - sz;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp->v.x + sz;
   *(float *)&wv[1] = tmp->v.y - sz;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp->v.x + sz;
   *(float *)&wv[1] = tmp->v.y + sz;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp->v.x + sz;
   *(float *)&wv[1] = tmp->v.y + sz;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp->v.x - sz;
   *(float *)&wv[1] = tmp->v.y + sz;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp->v.x - sz;
   *(float *)&wv[1] = tmp->v.y - sz;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp->ui[j];
}


static __inline void mga_draw_line( mgaContextPtr mmesa,
				  const mgaVertex *tmp0, 
				  const mgaVertex *tmp1,
				  float width )
{
   GLuint vertsize = mmesa->vertsize;
   GLuint *wv = mgaAllocVertexDwords( mmesa, 6 * VERTSIZE );
   float dx, dy, ix, iy;
   int j;

   dx = tmp0->v.x - tmp1->v.x;
   dy = tmp0->v.y - tmp1->v.y;

   ix = width * .5; iy = 0;
  
   if ((ix<.5) && (ix>0.1)) ix = .5; /* I want to see lines with width
                                        0.5 also */
  
   if (dx * dx > dy * dy) {
      iy = ix; ix = 0;
   }
  
   *(float *)&wv[0] = tmp0->v.x - ix;
   *(float *)&wv[1] = tmp0->v.y - iy;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp0->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp1->v.x + ix;
   *(float *)&wv[1] = tmp1->v.y + iy;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp1->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp0->v.x + ix;
   *(float *)&wv[1] = tmp0->v.y + iy;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp0->ui[j];
   wv += VERTSIZE;
	 
   *(float *)&wv[0] = tmp0->v.x - ix;
   *(float *)&wv[1] = tmp0->v.y - iy;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp0->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp1->v.x - ix;
   *(float *)&wv[1] = tmp1->v.y - iy;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp1->ui[j];
   wv += VERTSIZE;

   *(float *)&wv[0] = tmp1->v.x + ix;
   *(float *)&wv[1] = tmp1->v.y + iy;
   for (j = 2 ; j < vertsize ; j++) 
      wv[j] = tmp1->ui[j];
   wv += VERTSIZE;
}




#endif
