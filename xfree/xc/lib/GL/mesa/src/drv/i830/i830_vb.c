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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_vb.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */
 
#include <stdio.h>
#include <stdlib.h>

#include "i830_drv.h"
#include "mem.h"
#include "stages.h"

#define TEX0 {							\
  v->v.tu0 = tc0[i][0];						\
  v->v.tv0 = tc0[i][1];						\
}

#define TEX1 {					\
  v->v.tu1 = tc1[i][0];			\
  v->v.tv1 = tc1[i][1];			\
}

/* Doesn't seem to work very well (golly).
 */
#define SPC {					\
  GLubyte *spec = &(VB->Spec[0][i][0]);		\
  v->v.specular.red = spec[0];		\
  v->v.specular.green = spec[1];		\
  v->v.specular.blue = spec[2];		\
}

#define FOG {					\
  GLubyte *spec = &(VB->Spec[0][i][0]);		\
  v->v.specular.alpha = spec[3];		\
}

#define COL {					\
  GLubyte *col = &(VB->Color[0]->data[i][0]);	\
  v->v.color.blue  = col[2];		\
  v->v.color.green = col[1];		\
  v->v.color.red   = col[0];		\
  v->v.color.alpha = col[3];		\
}

/* The vertex formats we have don't seem to support projective texturing
 * in the multitexture case.  (Would require another 1/w value for the
 * second set of texcoords).    
 * XXX grantham - i830 does q coordinate, haven't spent time to figure out
 *  how to implement dynamic HOMOGENOUS coord; plus need to copy tc[i][2]
 *  for cube maps, not tc[i][3]
 */
#define TEX0_4						\
  if (VB->TexCoordPtr[0]->size == 4)			\
  {							\
     GLfloat (*tc)[4] = VB->TexCoordPtr[0]->data;	\
     v = &(I830_DRIVER_DATA(VB)->verts[start]);		\
     imesa->setupdone &= ~I830_WIN_BIT;			\
     for (i=start; i < end; i++, v++)	{		\
        float oow = 1.0 / tc[i][3];			\
	v->v.oow *= tc[i][3];				\
	v->v.tu0 *= oow;				\
	v->v.tv0 *= oow;				\
     }							\
  }

#define COORD							\
      GLfloat *win = VB->Win.data[i];				\
      v->v.x =       xoffset + win[0];				\
      v->v.y =       yoffset - win[1];				\
      v->v.z =       depth_scale * win[2];			\
      v->v.oow =               win[3];


#define NOP
#define SUBPIXEL_X 0.125
#define SUBPIXEL_Y 0.125

#define SETUPFUNC(name,win,col,tex0,tex1,tex0_4,spec,fog)		\
static void name(struct vertex_buffer *VB, GLuint start, GLuint end)	\
{									\
   i830ContextPtr imesa = I830_CONTEXT( VB->ctx );			\
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;			\
   i830VertexPtr v;							\
   GLfloat (*tc0)[4];							\
   GLfloat (*tc1)[4];							\
   const GLfloat depth_scale = imesa->depth_scale;			\
   const GLfloat xoffset = SUBPIXEL_X;					\
   const GLfloat yoffset = dPriv->h + SUBPIXEL_Y;			\
   int i;								\
   (void) xoffset;							\
   (void) yoffset;							\
   (void) imesa;							\
   (void) depth_scale;							\
									\
									\
   gl_import_client_data( VB, VB->ctx->RenderFlags,			\
			  (VB->ClipOrMask				\
			   ? VEC_WRITABLE|VEC_GOOD_STRIDE		\
			   : VEC_GOOD_STRIDE));				\
									\
   tc0 = VB->TexCoordPtr[0]->data;					\
   tc1 = VB->TexCoordPtr[1]->data;					\
									\
   v = &(I830_DRIVER_DATA(VB)->verts[start]);			\
									\
   if (VB->ClipOrMask == 0)						\
      for (i=start; i < end; i++, v++) {				\
	 win;								\
	 col;								\
	 spec;								\
	 fog;								\
	 tex0;								\
	 tex1;								\
      }									\
   else									\
      for (i=start; i < end; i++, v++) {				\
	 if (VB->ClipMask[i] == 0) {					\
	    win;							\
	    spec;							\
	    fog;							\
	    tex0;							\
	    tex1;							\
	 }								\
	    col;							\
      }									\
   tex0_4;								\
   /* tex1_4; */							\
   /* tex2_4; */							\
   /* tex3_4; */							\
}

 
SETUPFUNC(rs_wt0,	COORD,NOP,TEX0,NOP,TEX0_4,NOP,NOP)
SETUPFUNC(rs_wt0t1,	COORD,NOP,TEX0,TEX1,TEX0_4,NOP,NOP)
SETUPFUNC(rs_wft0,	COORD,NOP,TEX0,NOP,TEX0_4,NOP,FOG)
SETUPFUNC(rs_wft0t1,	COORD,NOP,TEX0,TEX1,TEX0_4,NOP,FOG)
SETUPFUNC(rs_wg,	COORD,COL,NOP,NOP,NOP,NOP,NOP)
SETUPFUNC(rs_wgs,	COORD,COL,NOP,NOP,NOP,SPC,NOP)
SETUPFUNC(rs_wgt0,	COORD,COL,TEX0,NOP,TEX0_4,NOP,NOP)
SETUPFUNC(rs_wgt0t1,	COORD,COL,TEX0,TEX1,TEX0_4,NOP,NOP)
SETUPFUNC(rs_wgst0,	COORD,COL,TEX0,NOP,TEX0_4,SPC,NOP)
SETUPFUNC(rs_wgst0t1,	COORD,COL,TEX0,TEX1,TEX0_4,SPC,NOP)
SETUPFUNC(rs_wgf,	COORD,COL,NOP,NOP,NOP,NOP,FOG)
SETUPFUNC(rs_wgfs,	COORD,COL,NOP,NOP,NOP,SPC,FOG)
SETUPFUNC(rs_wgft0,	COORD,COL,TEX0,NOP,TEX0_4,NOP,FOG)
SETUPFUNC(rs_wgft0t1,	COORD,COL,TEX0,TEX1,TEX0_4,NOP,FOG)
SETUPFUNC(rs_wgfst0,	COORD,COL,TEX0,NOP,TEX0_4,SPC,FOG)
SETUPFUNC(rs_wgfst0t1,	COORD,COL,TEX0,TEX1,TEX0_4,SPC,FOG)

SETUPFUNC(rs_t0,	NOP,NOP,TEX0,NOP,TEX0_4,NOP,NOP)
SETUPFUNC(rs_t0t1,	NOP,NOP,TEX0,TEX1,TEX0_4,NOP,NOP)
SETUPFUNC(rs_f,		NOP,NOP,NOP,NOP,NOP,NOP,FOG)
SETUPFUNC(rs_ft0,	NOP,NOP,TEX0,NOP,TEX0_4,NOP,FOG)
SETUPFUNC(rs_ft0t1,	NOP,NOP,TEX0,TEX1,TEX0_4,NOP,FOG)
SETUPFUNC(rs_g,		NOP,COL,NOP,NOP,NOP,NOP,NOP)
SETUPFUNC(rs_gs,	NOP,COL,NOP,NOP,NOP,SPC,NOP)
SETUPFUNC(rs_gt0,	NOP,COL,TEX0,NOP,TEX0_4,NOP,NOP)
SETUPFUNC(rs_gt0t1,	NOP,COL,TEX0,TEX1,TEX0_4,NOP,NOP)
SETUPFUNC(rs_gst0,	NOP,COL,TEX0,NOP,TEX0_4,SPC,NOP)
SETUPFUNC(rs_gst0t1,	NOP,COL,TEX0,TEX1,TEX0_4,SPC,NOP)
SETUPFUNC(rs_gf,	NOP,COL,NOP,NOP,NOP,NOP,FOG)
SETUPFUNC(rs_gfs,	NOP,COL,NOP,NOP,NOP,SPC,FOG)
SETUPFUNC(rs_gft0,	NOP,COL,TEX0,NOP,TEX0_4,NOP,FOG)
SETUPFUNC(rs_gft0t1,	NOP,COL,TEX0,TEX1,TEX0_4,NOP,FOG)
SETUPFUNC(rs_gfst0,	NOP,COL,TEX0,NOP,TEX0_4,SPC,FOG)
SETUPFUNC(rs_gfst0t1,	NOP,COL,TEX0,TEX1,TEX0_4,SPC,FOG)



static void rs_invalid(struct vertex_buffer *VB, GLuint start, GLuint end)
{
   
  fprintf(stderr, "i830RasterSetup(): invalid setup function\n");
}

typedef void (*setupFunc)(struct vertex_buffer *,GLuint,GLuint);

static setupFunc setup_func[0x80];

void i830DDSetupInit( void )
{
   int i;

   for (i = 0 ; i < 0x80 ; i++)
      setup_func[i] = rs_invalid;
   
   /* Functions to build vert's from scratch */
   setup_func[I830_WIN_BIT|I830_TEX0_BIT] = rs_wt0;
   setup_func[I830_WIN_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_wt0t1;
   setup_func[I830_WIN_BIT|I830_FOG_BIT|I830_TEX0_BIT] = rs_wft0;
   setup_func[I830_WIN_BIT|I830_FOG_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_wft0t1;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT] = rs_wg;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_SPEC_BIT] = rs_wgs;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_TEX0_BIT] = rs_wgt0;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_wgt0t1;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_SPEC_BIT|I830_TEX0_BIT] = rs_wgst0;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_SPEC_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_wgst0t1;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_FOG_BIT] = rs_wgf;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_FOG_BIT|I830_SPEC_BIT] = rs_wgfs;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_FOG_BIT|I830_TEX0_BIT] = rs_wgft0;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_FOG_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_wgft0t1;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_FOG_BIT|I830_SPEC_BIT|I830_TEX0_BIT] = rs_wgfst0;
   setup_func[I830_WIN_BIT|I830_RGBA_BIT|I830_FOG_BIT|I830_SPEC_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_wgfst0t1;

   /* Repair functions */
   setup_func[I830_TEX0_BIT] = rs_t0;
   setup_func[I830_TEX0_BIT|I830_TEX1_BIT] = rs_t0t1;
   setup_func[I830_FOG_BIT] = rs_f;
   setup_func[I830_FOG_BIT|I830_TEX0_BIT] = rs_ft0;
   setup_func[I830_FOG_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_ft0t1;
   setup_func[I830_RGBA_BIT] = rs_g;
   setup_func[I830_RGBA_BIT|I830_SPEC_BIT] = rs_gs;
   setup_func[I830_RGBA_BIT|I830_TEX0_BIT] = rs_gt0;
   setup_func[I830_RGBA_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_gt0t1;
   setup_func[I830_RGBA_BIT|I830_SPEC_BIT|I830_TEX0_BIT] = rs_gst0;
   setup_func[I830_RGBA_BIT|I830_SPEC_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_gst0t1;
   setup_func[I830_RGBA_BIT|I830_FOG_BIT] = rs_gf;
   setup_func[I830_RGBA_BIT|I830_FOG_BIT|I830_SPEC_BIT] = rs_gfs;
   setup_func[I830_RGBA_BIT|I830_FOG_BIT|I830_TEX0_BIT] = rs_gft0;
   setup_func[I830_RGBA_BIT|I830_FOG_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_gft0t1;
   setup_func[I830_RGBA_BIT|I830_FOG_BIT|I830_SPEC_BIT|I830_TEX0_BIT] = rs_gfst0;
   setup_func[I830_RGBA_BIT|I830_FOG_BIT|I830_SPEC_BIT|I830_TEX0_BIT|I830_TEX1_BIT] = rs_gfst0t1;

}


void i830PrintSetupFlags(char *msg, GLuint flags )
{
   fprintf(stderr, "%s: %d %s%s%s%s%s%s%s\n",
	   msg,
	   (int)flags,
	   (flags & I830_WIN_BIT)      ? " xyzw," : "", 
	   (flags & I830_RGBA_BIT)     ? " rgba," : "",
	   (flags & I830_SPEC_BIT)     ? " spec," : "",
	   (flags & I830_FOG_BIT)      ? " fog," : "",
	   (flags & I830_TEX0_BIT)     ? " tex-0," : "",
	   (flags & I830_TEX1_BIT)     ? " tex-1," : "",
	   (flags & I830_ALPHA_BIT)    ? " alpha," : "");
}


void i830ChooseRasterSetupFunc(GLcontext *ctx)
{
  i830ContextPtr imesa = I830_CONTEXT( ctx );
  int funcindex = (I830_WIN_BIT | I830_RGBA_BIT);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

  imesa->vertsize = 8;
  imesa->Setup[I830_CTXREG_VF] = VRTX_FORMAT_NTEX(1);

  if (ctx->Texture.ReallyEnabled & 0xf) 
     funcindex |= I830_TEX0_BIT;
  
  if (ctx->Texture.ReallyEnabled & 0xf0) {
     funcindex |= (I830_TEX0_BIT | I830_TEX1_BIT);
     imesa->vertsize = 10;
     imesa->Setup[I830_CTXREG_VF] = VRTX_FORMAT_NTEX(2);
  }

  if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
     funcindex |= I830_SPEC_BIT;

  if (ctx->FogMode == FOG_FRAGMENT)
     funcindex |= I830_FOG_BIT;
    
  if (MESA_VERBOSE)
     i830PrintSetupFlags("xsmesa: full setup function", funcindex); 

  imesa->setupindex = funcindex;
  ctx->Driver.RasterSetup = setup_func[funcindex];
   if(0) fprintf(stderr, "funcindex : 0x%x\n", funcindex);
}




void i830DDCheckPartialRasterSetup( GLcontext *ctx, 
				    struct gl_pipeline_stage *d )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   GLuint tmp = imesa->setupdone;

   d->type = 0;
   imesa->setupdone = 0;	/* cleared if we return */

   if ((ctx->Array.Summary & VERT_OBJ_ANY) == 0)
      return;

   if (ctx->IndirectTriangles) 
      return;

   imesa->setupdone = tmp;

   /* disabled until we have a merge&render op */
   /*     d->inputs = available; */
   /*     d->outputs = VERT_RAST_SETUP_PART; */
   /*     d->type = PIPE_PRECALC; */
}


/* Repair existing precalculated vertices with new data.
 */
void i830DDPartialRasterSetup( struct vertex_buffer *VB )
{
   i830ContextPtr imesa = I830_CONTEXT( VB->ctx );
   GLuint new = VB->pipeline->new_outputs;
   GLuint available = VB->pipeline->outputs;
   GLuint ind = 0;

   if (new & VERT_WIN) {
      new = available;
      ind |= I830_WIN_BIT | I830_FOG_BIT;
   }

   if (new & VERT_RGBA)
      ind |= I830_RGBA_BIT | I830_SPEC_BIT;

   if (new & VERT_TEX0_ANY) 
      ind |= I830_TEX0_BIT;

   if (new & VERT_TEX1_ANY)
      ind |= I830_TEX1_BIT;

   if (new & VERT_FOG_COORD)
      ind |= I830_FOG_BIT;

   imesa->setupdone &= ~ind;
   ind &= imesa->setupindex;
   imesa->setupdone |= ind;

   if (0) i830PrintSetupFlags("xsmesa: partial setup function", ind);

   if (ind) 
      setup_func[ind&~I830_ALPHA_BIT]( VB, VB->Start, VB->Count );   
}


void i830DDDoRasterSetup( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;

   if (VB->Type == VB_CVA_PRECALC) 
      i830DDPartialRasterSetup( VB );
   else if (ctx->Driver.RasterSetup)
      ctx->Driver.RasterSetup( VB, VB->CopyStart, VB->Count );
}




void i830DDResizeVB( struct vertex_buffer *VB, GLuint size )
{
   i830VertexBufferPtr mvb = I830_DRIVER_DATA(VB);

   while (mvb->size < size)
      mvb->size *= 2;

   free( mvb->vert_store );
   mvb->vert_store = malloc( sizeof(i830Vertex) * mvb->size + 31);
   if (!mvb->vert_store) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }

   mvb->verts = (i830VertexPtr)(((unsigned long)mvb->vert_store + 31) & ~31);

   gl_vector1ui_free( &mvb->clipped_elements );
   gl_vector1ui_alloc( &mvb->clipped_elements, VEC_WRITABLE, mvb->size, 32 );   
   if (!mvb->clipped_elements.start) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * mvb->size, 4);
   if (!VB->ClipMask) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }


   if (VB->Type == VB_IMMEDIATE) {
      free( mvb->primitive );
      free( mvb->next_primitive );
      mvb->primitive = (GLuint *)malloc( sizeof(GLuint) * mvb->size );
      mvb->next_primitive = (GLuint *)malloc( sizeof(GLuint) * mvb->size );
      if (!mvb->primitive || !mvb->next_primitive) {
	 fprintf(stderr, "i830glx: out of memory !\n");
	 exit(1);
      }
   }
}


void i830DDRegisterVB( struct vertex_buffer *VB )
{
   i830VertexBufferPtr mvb;

   mvb = (i830VertexBufferPtr)calloc( 1, sizeof(*mvb) );

   mvb->size = VB->Size * 2;
   mvb->vert_store = malloc( sizeof(i830Vertex) * mvb->size + 31);
   if (!mvb->vert_store) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }
   
   mvb->verts = (i830VertexPtr)(((unsigned long)mvb->vert_store + 31) & ~31);

   gl_vector1ui_alloc( &mvb->clipped_elements, VEC_WRITABLE, mvb->size, 32 );
   if (!mvb->clipped_elements.start) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }
      
   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * mvb->size, 4);
   if (!VB->ClipMask) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }

   mvb->primitive = (GLuint *)malloc( sizeof(GLuint) * mvb->size );
   mvb->next_primitive = (GLuint *)malloc( sizeof(GLuint) * mvb->size );
   if (!mvb->primitive || !mvb->next_primitive) {
      fprintf(stderr, "i830glx: out of memory !\n");
      exit(1);
   }

   VB->driver_data = mvb;
}


void i830DDUnregisterVB( struct vertex_buffer *VB )
{
   i830VertexBufferPtr mvb = I830_DRIVER_DATA(VB);
   
   if (mvb) {
      if (mvb->vert_store) free(mvb->vert_store);
      if (mvb->primitive) free(mvb->primitive);
      if (mvb->next_primitive) free(mvb->next_primitive);
      gl_vector1ui_free( &mvb->clipped_elements );
      free(mvb);
      VB->driver_data = 0;
   }      
}
