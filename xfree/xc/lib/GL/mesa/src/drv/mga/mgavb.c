/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgavb.c,v 1.11 2001/10/31 22:50:24 tsi Exp $ */

#include "mgacontext.h"
#include "mgavb.h"
#include "mga_xmesa.h"

#include "stages.h"
#include "mem.h"

#include <stdio.h>
#include <stdlib.h>

#define TEX0 {				\
  v->v.tu0 = tc0[i][0];			\
  v->v.tv0 = tc0[i][1];			\
}

#define TEX1 {				\
  v->v.tu1 = tc1[i][0];			\
  v->v.tv1 = tc1[i][1];			\
}

#define SPC {				\
  GLubyte *spec = &(VB->Spec[0][i][0]);	\
  v->v.specular.red = spec[0];		\
  v->v.specular.green = spec[1];	\
  v->v.specular.blue = spec[2];		\
}

#define FOG {				\
  GLubyte *spec = &(VB->Spec[0][i][0]);	\
  v->v.specular.alpha = spec[3];	\
}

#define COL {					\
  GLubyte *col = &(VB->Color[0]->data[i][0]);	\
  v->v.color.blue  = col[2];			\
  v->v.color.green = col[1];			\
  v->v.color.red   = col[0];			\
  v->v.color.alpha = col[3];			\
}

/* The v code we have doesn't seem to support projective texturing
 * in the multitexture case.  (Would require another 1/w value for the
 * second set of texcoords).  This may be a problem for the g400.
 */
#define TEX0_4						\
  if (VB->TexCoordPtr[0]->size == 4) {			\
     GLfloat (*tc)[4] = VB->TexCoordPtr[0]->data;	\
     v = &(MGA_DRIVER_DATA(VB)->verts[start]);		\
     mmesa->setupdone &= ~MGA_WIN_BIT;			\
     for (i = start; i < end; i++, v++) {		\
        GLfloat oow = 1.0 / tc[i][3];			\
	v->v.rhw *= tc[i][3];				\
	v->v.tu0 *= oow;				\
	v->v.tv0 *= oow;				\
     }							\
  }


#define COORD						\
      GLfloat *win = VB->Win.data[i];			\
      v->v.rhw =               win[3];			\
      v->v.z = depth_scale * win[2];			\
      v->v.x =                 win[0] + xoffset;	\
      v->v.y =          -      win[1] + yoffset;


#define NOP




#define SETUPFUNC(name,win,col,tex0,tex1,tex0_4,spec,fog)		\
static void name(struct vertex_buffer *VB, GLuint start, GLuint end)	\
{									\
   mgaContextPtr mmesa = MGA_CONTEXT( VB->ctx );                        \
   mgaVertexPtr v;							\
   GLfloat (*tc0)[4];							\
   GLfloat (*tc1)[4];							\
   const GLfloat depth_scale = mmesa->depth_scale;			\
   const GLfloat xoffset = mmesa->drawX + SUBPIXEL_X;			\
   const GLfloat yoffset = mmesa->driDrawable->h + mmesa->drawY + \
                           SUBPIXEL_Y;	\
   int i;								\
   (void) xoffset; (void) yoffset; (void) depth_scale;			\
   gl_import_client_data( VB, VB->ctx->RenderFlags,			\
			  (VB->ClipOrMask				\
			   ? VEC_WRITABLE|VEC_GOOD_STRIDE		\
			   : VEC_GOOD_STRIDE));				\
									\
   tc0 = VB->TexCoordPtr[mmesa->tmu_source[0]]->data;			\
   tc1 = VB->TexCoordPtr[mmesa->tmu_source[1]]->data;			\
									\
   v = &(MGA_DRIVER_DATA(VB)->verts[start]);				\
									\
   if (VB->ClipOrMask == 0)						\
      for (i=start; i < end; i++, v++) {				\
	 win;								\
	 col;								\
	 tex0;								\
	 tex1;								\
	 spec;								\
	 fog;								\
      }									\
   else									\
      for (i=start; i < end; i++, v++) {				\
	 if (VB->ClipMask[i] == 0) {					\
	    win;							\
	    tex0;							\
	    tex1;							\
	    spec;							\
	    fog;							\
	 }								\
         col;								\
      }									\
   tex0_4;								\
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
   fprintf(stderr, "mgaRasterSetup(): invalid combination\n");
}

typedef void (*setupFunc)(struct vertex_buffer *,GLuint,GLuint);

static setupFunc setup_func[0x80];

void mgaDDSetupInit( void )
{
   int i;

   for (i = 0 ; i < 0x80 ; i++)
      setup_func[i] = rs_invalid;

   /* Functions to build vert's from scratch */
   setup_func[MGA_WIN_BIT|MGA_TEX0_BIT] = rs_wt0;
   setup_func[MGA_WIN_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_wt0t1;
   setup_func[MGA_WIN_BIT|MGA_FOG_BIT|MGA_TEX0_BIT] = rs_wft0;
   setup_func[MGA_WIN_BIT|MGA_FOG_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_wft0t1;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT] = rs_wg;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_SPEC_BIT] = rs_wgs;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_TEX0_BIT] = rs_wgt0;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_wgt0t1;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT] = rs_wgst0;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_wgst0t1;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_FOG_BIT] = rs_wgf;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_FOG_BIT|MGA_SPEC_BIT] = rs_wgfs;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_FOG_BIT|MGA_TEX0_BIT] = rs_wgft0;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_FOG_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_wgft0t1;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_FOG_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT] = rs_wgfst0;
   setup_func[MGA_WIN_BIT|MGA_RGBA_BIT|MGA_FOG_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_wgfst0t1;

   /* Repair functions */
   setup_func[MGA_TEX0_BIT] = rs_t0;
   setup_func[MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_t0t1;
   setup_func[MGA_FOG_BIT] = rs_f;
   setup_func[MGA_FOG_BIT|MGA_TEX0_BIT] = rs_ft0;
   setup_func[MGA_FOG_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_ft0t1;
   setup_func[MGA_RGBA_BIT] = rs_g;
   setup_func[MGA_RGBA_BIT|MGA_SPEC_BIT] = rs_gs;
   setup_func[MGA_RGBA_BIT|MGA_TEX0_BIT] = rs_gt0;
   setup_func[MGA_RGBA_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_gt0t1;
   setup_func[MGA_RGBA_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT] = rs_gst0;
   setup_func[MGA_RGBA_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_gst0t1;
   setup_func[MGA_RGBA_BIT|MGA_FOG_BIT] = rs_gf;
   setup_func[MGA_RGBA_BIT|MGA_FOG_BIT|MGA_SPEC_BIT] = rs_gfs;
   setup_func[MGA_RGBA_BIT|MGA_FOG_BIT|MGA_TEX0_BIT] = rs_gft0;
   setup_func[MGA_RGBA_BIT|MGA_FOG_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_gft0t1;
   setup_func[MGA_RGBA_BIT|MGA_FOG_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT] = rs_gfst0;
   setup_func[MGA_RGBA_BIT|MGA_FOG_BIT|MGA_SPEC_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT] = rs_gfst0t1;

}


void mgaPrintSetupFlags(char *msg, GLuint flags )
{
   fprintf(stderr, "%s: %d %s%s%s%s%s%s%s\n",
	   msg,
	   (int)flags,
	   (flags & MGA_WIN_BIT)      ? " xyzw," : "",
	   (flags & MGA_RGBA_BIT)     ? " rgba," : "",
	   (flags & MGA_SPEC_BIT)     ? " spec," : "",
	   (flags & MGA_FOG_BIT)      ? " fog," : "",
	   (flags & MGA_TEX0_BIT)     ? " tex-0," : "",
	   (flags & MGA_TEX1_BIT)     ? " tex-1," : "",
	   (flags & MGA_ALPHA_BIT)    ? " alpha," : "");
}




void mgaChooseRasterSetupFunc(GLcontext *ctx)
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   int funcindex = (MGA_WIN_BIT | MGA_RGBA_BIT);
   int multi = mmesa->multitex;

   mmesa->vertsize = 8;
   mmesa->tmu_source[0] = 0;
   mmesa->tmu_source[1] = 1;
   mmesa->tex_dest[0] = MGA_TEX0_BIT;
   mmesa->tex_dest[1] = MGA_TEX1_BIT;
   mmesa->multitex = 0;
   mmesa->blend_flags &= ~MGA_BLEND_MULTITEX;

   if (ctx->Texture.ReallyEnabled & 0xf) {
      /* This doesn't work for non-RGBA textures
      if (ctx->Texture.Unit[0].EnvMode == GL_REPLACE)
	 funcindex &= ~MGA_RGBA_BIT;
      */
      if (ctx->Texture.Unit[0].EnvMode == GL_BLEND &&
	  mmesa->envcolor)
      {
	 mmesa->multitex = 1;
	 mmesa->vertsize = 10;
	 mmesa->tmu_source[1] = 0;
	 funcindex |= MGA_TEX1_BIT;
      }

      funcindex |= MGA_TEX0_BIT;
   }

   if (ctx->Texture.ReallyEnabled & 0xf0) {
      if (ctx->Texture.ReallyEnabled & 0xf) {
	 mmesa->multitex = 1;
	 mmesa->vertsize = 10;
	 mmesa->blend_flags |= MGA_BLEND_MULTITEX;
	 funcindex |= MGA_TEX1_BIT;
      } else {
	 /* Just a funny way of doing single texturing
	  */
	 mmesa->tmu_source[0] = 1;
	 mmesa->tex_dest[1] = MGA_TEX0_BIT;

	 if (ctx->Texture.Unit[0].EnvMode == GL_BLEND &&
	     mmesa->envcolor)
	 {
	    mmesa->multitex = 1;
	    mmesa->vertsize = 10;
	    mmesa->tmu_source[1] = 1;
	    funcindex |= MGA_TEX1_BIT;
	 }

	 funcindex |= MGA_TEX0_BIT;
      }
   }

   if (multi != mmesa->multitex)
        mmesa->new_state |= MGA_NEW_WARP;


   /* Not really a good place to do this - need to make the mga state
    * management code more event-driven so this can be calculated for
    * free.
    */

   if (ctx->Color.BlendEnabled)
      funcindex |= MGA_ALPHA_BIT;

   if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
      funcindex |= MGA_SPEC_BIT;

   if (ctx->Fog.Enabled)
      funcindex |= MGA_FOG_BIT;

   if (0)
      mgaPrintSetupFlags("xsmesa: full setup function", funcindex);

   mmesa->dirty |= MGA_UPLOAD_PIPE;
   mmesa->setupindex = funcindex;

   /* Called by mesa's clip functons:
    */
   ctx->Driver.RasterSetup = setup_func[funcindex & ~MGA_ALPHA_BIT];
}




void mgaDDCheckPartialRasterSetup( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   GLuint tmp = mmesa->setupdone;

   d->type = 0;
   mmesa->setupdone = 0;	/* cleared if we return */

   if ((ctx->Array.Summary & VERT_OBJ_ANY) == 0)
      return;

   if (ctx->IndirectTriangles)
      return;

   mmesa->setupdone = tmp;

   /* disabled until we have a merge&render op */
   /*     d->inputs = available; */
   /*     d->outputs = VERT_RAST_SETUP_PART; */
   /*     d->type = PIPE_PRECALC; */
}


/* Repair existing precalculated vertices with new data.
 */
void mgaDDPartialRasterSetup( struct vertex_buffer *VB )
{
   mgaContextPtr mmesa = MGA_CONTEXT( VB->ctx );
   GLuint new = VB->pipeline->new_outputs;
   GLuint available = VB->pipeline->outputs;
   GLuint ind = 0;

   if (new & VERT_WIN) {
      new = available;
      ind |= MGA_WIN_BIT | MGA_FOG_BIT;
   }

   if (new & VERT_RGBA)
      ind |= MGA_RGBA_BIT | MGA_SPEC_BIT;

   if (new & VERT_TEX0_ANY)
      ind |= MGA_TEX0_BIT;

   if (new & VERT_TEX1_ANY)
      ind |= mmesa->tex_dest[1];

   if (new & VERT_FOG_COORD)
      ind |= MGA_FOG_BIT;

   mmesa->setupdone &= ~ind;
   ind &= mmesa->setupindex;
   mmesa->setupdone |= ind;

   if (0)
      mgaPrintSetupFlags("xsmesa: partial setup function", ind);

   if (ind)
      setup_func[ind&~MGA_ALPHA_BIT]( VB, VB->Start, VB->Count );
}


void mgaDDDoRasterSetup( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
/*     mgaContextPtr mmesa = MGA_CONTEXT( ctx ); */

   /* Can't lock, won't lock
    */
/*     REFRESH_DRAWABLE_INFO( mmesa ); */

   if (VB->Type == VB_CVA_PRECALC)
      mgaDDPartialRasterSetup( VB );
   else if (ctx->Driver.RasterSetup)
      ctx->Driver.RasterSetup( VB, VB->CopyStart, VB->Count );
}

static void FatalError( char *s )
{
   fprintf(stderr, s);
   exit(1);
}


void mgaDDResizeVB( struct vertex_buffer *VB, GLuint size )
{
   mgaVertexBufferPtr mvb = MGA_DRIVER_DATA(VB);

   while (mvb->size < size)
      mvb->size *= 2;

   FREE( mvb->vert_store );
   mvb->vert_store = MALLOC( sizeof(mgaVertex) * mvb->size + 31);
   if (!mvb->vert_store)
      FatalError("mga-glx: out of memory !\n");

   mvb->verts = (mgaVertexPtr)(((unsigned long)mvb->vert_store + 31) & ~31);

   gl_vector1ui_free( &mvb->clipped_elements );
   gl_vector1ui_alloc( &mvb->clipped_elements, VEC_WRITABLE, mvb->size, 32 );
   if (!mvb->clipped_elements.start)
      FatalError("mga-glx: out of memory !\n");

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *)ALIGN_MALLOC(sizeof(GLubyte) * mvb->size, 32);
   if (!VB->ClipMask)
      FatalError("mga-glx: out of memory !\n");

   if (VB->Type == VB_IMMEDIATE) {
      FREE( mvb->primitive );
      FREE( mvb->next_primitive );
      mvb->primitive = (GLuint *)MALLOC( sizeof(GLuint) * mvb->size );
      mvb->next_primitive = (GLuint *)MALLOC( sizeof(GLuint) * mvb->size );
      if (!mvb->primitive || !mvb->next_primitive)
	 FatalError("mga-glx: out of memory!");
   }
}


void mgaDDRegisterVB( struct vertex_buffer *VB )
{
   mgaVertexBufferPtr mvb;

   mvb = (mgaVertexBufferPtr)MALLOC( sizeof(*mvb) );

   /* This looks like it allocates a lot of memory, but it basically
    * just sets an upper limit on how much can be used - nothing like
    * this amount will ever be turned into 'real' memory.
    */
   mvb->size = VB->Size * 5;
   mvb->vert_store = MALLOC( sizeof(mgaVertex) * mvb->size + 31);
   if (!mvb->vert_store)
      FatalError("mga-glx: out of memory !\n");

   mvb->verts = (mgaVertexPtr)(((unsigned long)mvb->vert_store + 31) & ~31);

   gl_vector1ui_alloc( &mvb->clipped_elements, VEC_WRITABLE, mvb->size, 32 );
   if (!mvb->clipped_elements.start)
      FatalError("mga-glx: out of memory !\n");

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *)ALIGN_MALLOC(sizeof(GLubyte) * mvb->size, 32);
   if (!VB->ClipMask)
      FatalError("mga-glx: out of memory !\n");

   mvb->primitive = (GLuint *)MALLOC( sizeof(GLuint) * mvb->size );
   mvb->next_primitive = (GLuint *)MALLOC( sizeof(GLuint) * mvb->size );
   if (!mvb->primitive || !mvb->next_primitive)
      FatalError("mga-glx: out of memory!");

   VB->driver_data = mvb;
}


void mgaDDUnregisterVB( struct vertex_buffer *VB )
{
   mgaVertexBufferPtr mvb = MGA_DRIVER_DATA(VB);

   if (mvb) {
      if (mvb->vert_store) FREE(mvb->vert_store);
      if (mvb->primitive) FREE(mvb->primitive);
      if (mvb->next_primitive) FREE(mvb->next_primitive);
      gl_vector1ui_free( &mvb->clipped_elements );
      FREE(mvb);
      VB->driver_data = 0;
   }
}
