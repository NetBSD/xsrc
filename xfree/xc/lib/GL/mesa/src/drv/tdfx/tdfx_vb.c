/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_vb.c,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

#include "tdfx_context.h"
#include "tdfx_vb.h"

#include "stages.h"
#include "mem.h"


#define COORD								\
do {									\
   v->v.x   = winCoord[0] + xoffset;					\
   v->v.y   = winCoord[1] + yoffset;					\
   v->v.z   = winCoord[2];						\
   v->v.rhw = w;							\
} while (0)


/* The assembly is slower...
 */
#if 0 && defined(USE_X86_ASM)
#define COL								\
do {									\
   __asm__ (								\
      "movl (%%edx),%%eax   \n"						\
      "bswap %%eax          \n"						\
      "rorl $8,%%eax        \n"						\
      "movl %%eax,16(%%edi) \n"						\
      :									\
      : "d" (color), "D" (v)						\
      : "%eax" );							\
} while (0)
#else
#define COL								\
do {									\
   v->v.color.blue  = color[2];						\
   v->v.color.green = color[1];						\
   v->v.color.red   = color[0];						\
   v->v.color.alpha = color[3];						\
} while (0)
#endif


#define TEX0								\
do {									\
   v->v.tu0 = tc0[i][0] * sScale0 * w;					\
   v->v.tv0 = tc0[i][1] * tScale0 * w;					\
} while (0)

#define TEX1								\
do {									\
   v->v.tu1 = tc1[i][0] * sScale1 * w;					\
   v->v.tv1 = tc1[i][1] * tScale1 * w;					\
} while (0)


#define TEX0_4								\
   if ( VB->TexCoordPtr[0]->size == 4 ) {				\
      GLfloat (*tc)[4] = VB->TexCoordPtr[0]->data;			\
      winCoord = VB->Win.data[start];					\
      v = &(TDFX_DRIVER_DATA(VB)->verts[start]);			\
      for ( i = start ; i < end ; i++, v++, winCoord+=4 )  {		\
	 v->v.tq0 = tc[i][3] * winCoord[3];				\
      }									\
   }

#define TEX1_4								\
   if ( VB->TexCoordPtr[1]->size == 4 ) {				\
      GLfloat (*tc)[4] = VB->TexCoordPtr[1]->data;			\
      winCoord = VB->Win.data[start];					\
      v = &(TDFX_DRIVER_DATA(VB)->verts[start]);			\
      for ( i = start ; i < end ; i++, v++, winCoord+=4 )  {		\
	 v->v.tq1 = tc[i][3] * winCoord[3];				\
      }									\
   }


#define FOG


#define NOP



#define SETUPFUNC(name,win,col,tex0,tex1,tex0_4,tex1_4,fog)		\
static void name( struct vertex_buffer *VB, GLuint start, GLuint end )	\
{									\
   tdfxContextPtr fxMesa = TDFX_CONTEXT(VB->ctx);			\
   tdfxVertexPtr v;							\
   const GLfloat *winCoord;						\
   GLfloat (*tc0)[4];							\
   GLfloat (*tc1)[4];							\
   const GLfloat xoffset = fxMesa->x_offset + TRI_X_OFFSET;		\
   const GLfloat yoffset = fxMesa->y_delta + TRI_Y_OFFSET;		\
   const GLfloat sScale0 = fxMesa->sScale0;				\
   const GLfloat tScale0 = fxMesa->tScale0;				\
   const GLfloat sScale1 = fxMesa->sScale1;				\
   const GLfloat tScale1 = fxMesa->tScale1;				\
   const GLubyte *color;						\
   int i;								\
									\
   (void) xoffset; (void) yoffset;					\
   (void) sScale0; (void) tScale0;					\
   (void) sScale1; (void) tScale1;					\
									\
   if (0) fprintf(stderr, "%s\n", __FUNCTION__);			\
   gl_import_client_data( VB, VB->ctx->RenderFlags,			\
			  (VB->ClipOrMask				\
			   ? VEC_WRITABLE|VEC_GOOD_STRIDE		\
			   : VEC_GOOD_STRIDE));				\
									\
   tc0 = VB->TexCoordPtr[fxMesa->tmu_source[0]]->data;			\
   tc1 = VB->TexCoordPtr[fxMesa->tmu_source[1]]->data;			\
   color = VB->Color[0]->data[start];					\
   winCoord = VB->Win.data[start];					\
									\
   v = &(TDFX_DRIVER_DATA(VB)->verts[start]);				\
									\
   if ( VB->ClipOrMask == 0 ) {						\
      for ( i = start ; i < end ; i++, v++, color+=4, winCoord+=4 ) {	\
         const GLfloat w = winCoord[3];					\
         (void) w;							\
	 win;								\
	 col;								\
	 fog;								\
         tex0;								\
         tex1;								\
      }									\
   } else {								\
      for ( i = start ; i < end ; i++, v++, color+=4, winCoord+=4 ) {	\
	 if ( VB->ClipMask[i] == 0 ) {					\
            const GLfloat w = winCoord[3];				\
	    (void) w;							\
	    win;							\
	    fog;							\
	    tex0;							\
	    tex1;							\
	 }								\
	 col;								\
      }									\
   }									\
   tex0_4;								\
   tex1_4;								\
}


SETUPFUNC(rs_wt0,	COORD, NOP, TEX0, NOP,  TEX0_4, NOP,    NOP)
SETUPFUNC(rs_wt0t1,	COORD, NOP, TEX0, TEX1, TEX0_4, TEX1_4, NOP)
SETUPFUNC(rs_wft0,	COORD, NOP, TEX0, NOP,  TEX0_4, NOP,    FOG)
SETUPFUNC(rs_wft0t1,	COORD, NOP, TEX0, TEX1, TEX0_4, TEX1_4, FOG)
SETUPFUNC(rs_wg,	COORD, COL, NOP,  NOP,  NOP,    NOP,    NOP)
SETUPFUNC(rs_wgt0,	COORD, COL, TEX0, NOP,  TEX0_4, NOP,    NOP)
SETUPFUNC(rs_wgt0t1,	COORD, COL, TEX0, TEX1, TEX0_4, TEX1_4, NOP)
SETUPFUNC(rs_wgf,	COORD, COL, NOP,  NOP,  NOP,    NOP,    FOG)
SETUPFUNC(rs_wgft0,	COORD, COL, TEX0, NOP,  TEX0_4, NOP,    FOG)
SETUPFUNC(rs_wgft0t1,	COORD, COL, TEX0, TEX1, TEX0_4, TEX1_4, FOG)

SETUPFUNC(rs_t0,	NOP,   NOP, TEX0, NOP,  TEX0_4, NOP,    NOP)
SETUPFUNC(rs_t0t1,	NOP,   NOP, TEX0, TEX1, TEX0_4, TEX1_4, NOP)
SETUPFUNC(rs_f,		NOP,   NOP, NOP,  NOP,  NOP,    NOP,    FOG)
SETUPFUNC(rs_ft0,	NOP,   NOP, TEX0, NOP,  TEX0_4, NOP,    FOG)
SETUPFUNC(rs_ft0t1,	NOP,   NOP, TEX0, TEX1, TEX0_4, TEX1_4, FOG)
SETUPFUNC(rs_g,		NOP,   COL, NOP,  NOP,  NOP,    NOP,    NOP)
SETUPFUNC(rs_gt0,	NOP,   COL, TEX0, NOP,  TEX0_4, NOP,    NOP)
SETUPFUNC(rs_gt0t1,	NOP,   COL, TEX0, TEX1, TEX0_4, TEX1_4, NOP)
SETUPFUNC(rs_gf,	NOP,   COL, NOP,  NOP,  NOP,    NOP,    FOG)
SETUPFUNC(rs_gft0,	NOP,   COL, TEX0, NOP,  TEX0_4, NOP,    FOG)
SETUPFUNC(rs_gft0t1,	NOP,   COL, TEX0, TEX1, TEX0_4, TEX1_4, FOG)



static void rs_invalid( struct vertex_buffer *VB, GLuint start, GLuint end )
{
   fprintf( stderr, "tdfxRasterSetup(): invalid setup function\n" );
}

typedef void (*setupFunc)( struct vertex_buffer *, GLuint, GLuint );

static setupFunc setup_func[0x40];


void tdfxDDSetupInit( void )
{
   int i;

   for (i = 0; i < Elements(setup_func); i++)
      setup_func[i] = rs_invalid;

   /* Functions to build vertices from scratch */
   setup_func[TDFX_WIN_BIT|TDFX_TEX0_BIT] = rs_wt0;
   setup_func[TDFX_WIN_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_wt0t1;
   setup_func[TDFX_WIN_BIT|TDFX_FOG_BIT|TDFX_TEX0_BIT] = rs_wft0;
   setup_func[TDFX_WIN_BIT|TDFX_FOG_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_wft0t1;
   setup_func[TDFX_WIN_BIT|TDFX_RGBA_BIT] = rs_wg;
   setup_func[TDFX_WIN_BIT|TDFX_RGBA_BIT|TDFX_TEX0_BIT] = rs_wgt0;
   setup_func[TDFX_WIN_BIT|TDFX_RGBA_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_wgt0t1;
   setup_func[TDFX_WIN_BIT|TDFX_RGBA_BIT|TDFX_FOG_BIT] = rs_wgf;
   setup_func[TDFX_WIN_BIT|TDFX_RGBA_BIT|TDFX_FOG_BIT|TDFX_TEX0_BIT] = rs_wgft0;
   setup_func[TDFX_WIN_BIT|TDFX_RGBA_BIT|TDFX_FOG_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_wgft0t1;

   /* Repair functions */
   setup_func[TDFX_TEX0_BIT] = rs_t0;
   setup_func[TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_t0t1;
   setup_func[TDFX_FOG_BIT] = rs_f;
   setup_func[TDFX_FOG_BIT|TDFX_TEX0_BIT] = rs_ft0;
   setup_func[TDFX_FOG_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_ft0t1;
   setup_func[TDFX_RGBA_BIT] = rs_g;
   setup_func[TDFX_RGBA_BIT|TDFX_TEX0_BIT] = rs_gt0;
   setup_func[TDFX_RGBA_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_gt0t1;
   setup_func[TDFX_RGBA_BIT|TDFX_FOG_BIT] = rs_gf;
   setup_func[TDFX_RGBA_BIT|TDFX_FOG_BIT|TDFX_TEX0_BIT] = rs_gft0;
   setup_func[TDFX_RGBA_BIT|TDFX_FOG_BIT|TDFX_TEX0_BIT|TDFX_TEX1_BIT] = rs_gft0t1;
}


void tdfxPrintSetupFlags( char *msg, GLuint flags )
{
   fprintf( stderr, "%s: 0x%x %s%s%s%s%s\n",
	    msg,
	    (int)flags,
	    (flags & TDFX_WIN_BIT)	? " xyzw," : "",
	    (flags & TDFX_RGBA_BIT)	? " rgba," : "",
	    (flags & TDFX_FOG_BIT)	? " fog," : "",
	    (flags & TDFX_TEX0_BIT)	? " tex-0," : "",
	    (flags & TDFX_TEX1_BIT)	? " tex-1," : "" );
}


/* ================================================================
 * Raster Setup
 */

void tdfxDDChooseRasterSetupFunc( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   int index = TDFX_WIN_BIT | TDFX_RGBA_BIT;
   int vertexFormat = fxMesa->vertexFormat;

   fxMesa->vertsize = 8;
   fxMesa->tmu_source[0] = 0;
   fxMesa->tmu_source[1] = 1;
   fxMesa->tex_dest[0] = TDFX_TEX0_BIT;
   fxMesa->tex_dest[1] = TDFX_TEX1_BIT;
   fxMesa->vertexFormat = TDFX_LAYOUT_SINGLE;

   if ( ctx->Texture.ReallyEnabled & ENABLE_TEX0 ) {
      index |= TDFX_TEX0_BIT;
   }

   if ( ctx->Texture.ReallyEnabled & ENABLE_TEX1 ) {
      if ( ctx->Texture.ReallyEnabled & ENABLE_TEX0 ) {
	 fxMesa->vertexFormat = TDFX_LAYOUT_MULTI;
	 fxMesa->vertsize = 10;
	 index |= TDFX_TEX1_BIT;
      } else {
	 /* Just a funny way of doing single texturing.
	  */
	 fxMesa->tmu_source[0] = 1;
	 fxMesa->tex_dest[1] = TDFX_TEX0_BIT;
	 index |= TDFX_TEX0_BIT;
      }
   }

   if (ctx->Texture.ReallyEnabled & (ENABLE_TEX0 | ENABLE_TEX1)) {
      if ((ctx->VB->TexCoordPtr[0] && ctx->VB->TexCoordPtr[0]->size == 4) ||
          (ctx->VB->TexCoordPtr[1] && ctx->VB->TexCoordPtr[1]->size == 4)) {
         fxMesa->vertexFormat = TDFX_LAYOUT_PROJECT;
      }
   }

   if ( ctx->Fog.Enabled )
      index |= TDFX_FOG_BIT;

   fxMesa->SetupIndex = index;
   ctx->Driver.RasterSetup = setup_func[index];

   if ( fxMesa->vertexFormat != vertexFormat ) {
      fxMesa->dirty |= TDFX_UPLOAD_VERTEX_LAYOUT;
   }

   if (0) {
      tdfxPrintSetupFlags( "full setup function", index );
      fprintf(stderr, "full setup function %p\n", ctx->Driver.RasterSetup);
   }
}

/* Check to see if vertices need repairing.
 */
void tdfxDDCheckPartialRasterSetup( GLcontext *ctx,
				    struct gl_pipeline_stage *d )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   int tmp = fxMesa->SetupDone;

   d->type = 0;
   fxMesa->SetupDone = 0;	/* cleared if we return */

   if ( (ctx->Array.Summary & VERT_OBJ_ANY) == 0 )
      return;
   if ( ctx->IndirectTriangles )
      return;

   fxMesa->SetupDone = tmp;
}

/* Repair existing precalculated vertices with new data.
 */
void tdfxDDPartialRasterSetup( struct vertex_buffer *VB )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( VB->ctx );
   GLuint new = VB->pipeline->new_outputs;
   GLuint available = VB->pipeline->outputs;
   GLuint index = 0;

   if ( new & VERT_WIN ) {
      new = available;
      index |= TDFX_WIN_BIT | TDFX_FOG_BIT;
   }

   if ( new & VERT_RGBA )
      index |= TDFX_RGBA_BIT;

   if ( new & VERT_TEX0_ANY )
      index |= TDFX_TEX0_BIT;

   if ( new & VERT_TEX1_ANY )
      index |= fxMesa->tex_dest[1];

   if ( new & VERT_FOG_COORD )
      index |= TDFX_FOG_BIT;

   fxMesa->SetupDone &= ~index;
   index &= fxMesa->SetupIndex;
   fxMesa->SetupDone |= index;

   if ( 0 )
      tdfxPrintSetupFlags( "partial setup function", index );

   if ( index )
      setup_func[index]( VB, VB->Start, VB->Count );
}

void tdfxDDDoRasterSetup( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;

   if ( VB->Type == VB_CVA_PRECALC ) {
      tdfxDDPartialRasterSetup( VB );
   } else if ( ctx->Driver.RasterSetup ) {
      ctx->Driver.RasterSetup( VB, VB->CopyStart, VB->Count );
   }
}


/* ================================================================
 * Device-specific Vertex Buffers
 */

void tdfxDDResizeVB( struct vertex_buffer *VB, GLuint size )
{
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);

   while ( fxVB->size < size )
      fxVB->size *= 2;

   ALIGN_FREE( fxVB->vert_store );
   fxVB->vert_store = ALIGN_MALLOC( sizeof(tdfxVertex) * fxVB->size, 32 );
   if ( !fxVB->vert_store ) {
      fprintf( stderr, "Cannot allocate vertex store!  Exiting...\n" );
      exit( 1 );
   }

   fxVB->verts = (tdfxVertexPtr)fxVB->vert_store;

   gl_vector1ui_free( &fxVB->clipped_elements );
   gl_vector1ui_alloc( &fxVB->clipped_elements,
		       VEC_WRITABLE, fxVB->size, 32 );
   if ( !fxVB->clipped_elements.start ) {
      fprintf( stderr, "Cannot allocate clipped elements!  Exiting...\n" );
      exit( 1 );
   }

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *)ALIGN_MALLOC( sizeof(GLubyte) * fxVB->size, 32 );
   if ( !VB->ClipMask ) {
      fprintf( stderr, "Cannot allocate clipmask!  Exiting...\n" );
      exit( 1 );
   }
}

void tdfxDDResizeElts( struct vertex_buffer *VB, GLuint size )
{
#if 0
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);

   FREE(fxVB->elts);

   while (fxVB->elt_size < size)
      fxVB->elt_size *= 2;

   FREE(fxVB->elts);
   fxVB->elts = MALLOC( sizeof(tdfxVertex *) * fxVB->elt_size );
   if ( !fxVB->elts ) {
      fprintf( stderr, "Cannot allocate vertex indirection!  Exiting...\n" );
      exit( 1 );
   }
#endif
}

void tdfxDDRegisterVB( struct vertex_buffer *VB )
{
   tdfxVertexBufferPtr fxVB;

   fxVB = (tdfxVertexBufferPtr) CALLOC( sizeof(*fxVB) );

   fxVB->elt_size = fxVB->size = VB->Size * 2;
   fxVB->vert_store = ALIGN_MALLOC( sizeof(tdfxVertex) * fxVB->size, 32 );
   if ( !fxVB->vert_store ) {
      fprintf( stderr, "Cannot allocate vertex store!  Exiting...\n" );
      exit( 1 );
   }

   fxVB->verts = (tdfxVertexPtr)fxVB->vert_store;

#if 0
   fxVB->elts = MALLOC( sizeof(tdfxVertex *) * fxVB->elt_size );
   if ( !fxVB->elts ) {
      fprintf( stderr, "Cannot allocate vertex indirection!  Exiting...\n" );
      exit( 1 );
   }
#endif

   gl_vector1ui_alloc( &fxVB->clipped_elements,
		       VEC_WRITABLE, fxVB->size, 32 );
   if ( !fxVB->clipped_elements.start ) {
      fprintf( stderr, "Cannot allocate clipped elements!  Exiting...\n" );
      exit( 1 );
   }


   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *)ALIGN_MALLOC( sizeof(GLubyte) * fxVB->size, 32 );
   if ( !VB->ClipMask ) {
      fprintf( stderr, "Cannot allocate clipmask!  Exiting...\n" );
      exit( 1 );
   }

   VB->driver_data = fxVB;
}

void tdfxDDUnregisterVB( struct vertex_buffer *VB )
{
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);

   if ( fxVB ) {
      if ( fxVB->vert_store ) ALIGN_FREE( fxVB->vert_store );
      if ( fxVB->elts ) ALIGN_FREE( fxVB->elts );
      gl_vector1ui_free( &fxVB->clipped_elements );
      FREE( fxVB );
      VB->driver_data = NULL;
   }
}
