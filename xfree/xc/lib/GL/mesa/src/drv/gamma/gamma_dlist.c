/*
 * Mesa 3-D graphics library
 * Version:  3.0
 * Copyright (C) 1995-1998  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* 
 * Pinched from Mesa v3.0 to glue into gamma driver
 * then glued up, to match some of the Mesa v3.3 back end.
 * Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_dlist.c,v 1.3 2001/02/12 01:11:24 tsi Exp $*/

#ifdef PC_HEADER
#include "all.h"
#else
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "accum.h"
#include "alpha.h"
#include "attrib.h"
#include "bitmap.h"
#include "blend.h"
#include "clip.h"
#include "colortab.h"
#include "context.h"
#include "copypix.h"
#include "depth.h"
#include "drawpix.h"
#include "enable.h"
#include "eval.h"
#include "feedback.h"
#include "fog.h"
#include "hash.h"
#include "image.h"
#include "light.h"
#include "lines.h"
#include "logic.h"
#include "macros.h"
#include "masking.h"
#include "matrix.h"
#include "mem.h"
#include "pixel.h"
#include "points.h"
#include "polygon.h"
#include "rastpos.h"
#include "rect.h"
#include "scissor.h"
#include "stencil.h"
#include "texobj.h"
#include "teximage.h"
#include "texstate.h"
#include "types.h"
#include "vb.h"
#include "vbfill.h"
#include "winpos.h"
#endif

#include "gamma_gl.h"
#include "gamma_init.h"
#include "gamma_dlist.h"


/*
Functions which aren't compiled but executed immediately:
	glIsList
	glGenLists
	glDeleteLists
	glEndList
	glFeedbackBuffer
	glSelectBuffer
	glRenderMode
	glReadPixels
	glPixelStore
	glFlush
	glFinish
	glIsEnabled
	glGet*

Functions which cause errors if called while compiling a display list:
	glNewList
*/



/*
 * Display list instructions are stored as sequences of "nodes".  Nodes
 * are allocated in blocks.  Each block has BLOCK_SIZE nodes.  Blocks
 * are linked together with a pointer.
 */


/* How many nodes to allocate at a time: */
#define BLOCK_SIZE 500


/*
 * Display list opcodes.
 *
 * The fact that these identifiers are assigned consecutive
 * integer values starting at 0 is very important, see InstSize array usage)
 */
typedef enum {
	OPCODE_ACCUM,
	OPCODE_ALPHA_FUNC,
        OPCODE_BEGIN,
        OPCODE_BIND_TEXTURE,
	OPCODE_BITMAP,
	OPCODE_BLEND_FUNC,
        OPCODE_CALL_LIST,
        OPCODE_CALL_LIST_OFFSET,
	OPCODE_CLEAR,
	OPCODE_CLEAR_ACCUM,
	OPCODE_CLEAR_COLOR,
	OPCODE_CLEAR_DEPTH,
	OPCODE_CLEAR_INDEX,
	OPCODE_CLEAR_STENCIL,
        OPCODE_CLIP_PLANE,
	OPCODE_COLOR_3F,
	OPCODE_COLOR_4F,
	OPCODE_COLOR_4UB,
	OPCODE_COLOR_MASK,
	OPCODE_COLOR_MATERIAL,
	OPCODE_COPY_PIXELS,
        OPCODE_COPY_TEX_IMAGE1D,
        OPCODE_COPY_TEX_IMAGE2D,
        OPCODE_COPY_TEX_SUB_IMAGE1D,
        OPCODE_COPY_TEX_SUB_IMAGE2D,
	OPCODE_CULL_FACE,
	OPCODE_DEPTH_FUNC,
	OPCODE_DEPTH_MASK,
	OPCODE_DEPTH_RANGE,
	OPCODE_DISABLE,
	OPCODE_DRAW_BUFFER,
	OPCODE_DRAW_PIXELS,
        OPCODE_EDGE_FLAG,
	OPCODE_ENABLE,
        OPCODE_END,
	OPCODE_EVALCOORD1,
	OPCODE_EVALCOORD2,
	OPCODE_EVALMESH1,
	OPCODE_EVALMESH2,
	OPCODE_EVALPOINT1,
	OPCODE_EVALPOINT2,
	OPCODE_FOG,
	OPCODE_FRONT_FACE,
	OPCODE_FRUSTUM,
	OPCODE_HINT,
	OPCODE_INDEX,
	OPCODE_INDEX_MASK,
	OPCODE_INIT_NAMES,
	OPCODE_LIGHT,
	OPCODE_LIGHT_MODEL,
	OPCODE_LINE_STIPPLE,
	OPCODE_LINE_WIDTH,
	OPCODE_LIST_BASE,
	OPCODE_LOAD_IDENTITY,
	OPCODE_LOAD_MATRIX,
	OPCODE_LOAD_NAME,
	OPCODE_LOGIC_OP,
	OPCODE_MAP1,
	OPCODE_MAP2,
	OPCODE_MAPGRID1,
	OPCODE_MAPGRID2,
	OPCODE_MATERIAL,
	OPCODE_MATRIX_MODE,
	OPCODE_MULT_MATRIX,
        OPCODE_NORMAL,
	OPCODE_ORTHO,
	OPCODE_PASSTHROUGH,
	OPCODE_PIXEL_MAP,
	OPCODE_PIXEL_TRANSFER,
	OPCODE_PIXEL_ZOOM,
	OPCODE_POINT_SIZE,
	OPCODE_POLYGON_MODE,
        OPCODE_POLYGON_STIPPLE,
	OPCODE_POLYGON_OFFSET,
	OPCODE_POP_ATTRIB,
	OPCODE_POP_MATRIX,
	OPCODE_POP_NAME,
	OPCODE_PRIORITIZE_TEXTURE,
	OPCODE_PUSH_ATTRIB,
	OPCODE_PUSH_MATRIX,
	OPCODE_PUSH_NAME,
	OPCODE_RASTER_POS,
	OPCODE_RECTF,
	OPCODE_READ_BUFFER,
        OPCODE_SCALE,
	OPCODE_SCISSOR,
	OPCODE_SHADE_MODEL,
	OPCODE_STENCIL_FUNC,
	OPCODE_STENCIL_MASK,
	OPCODE_STENCIL_OP,
	OPCODE_TEXCOORD2,
	OPCODE_TEXCOORD4,
        OPCODE_TEXENV,
        OPCODE_TEXGEN,
        OPCODE_TEXPARAMETER,
	OPCODE_TEX_IMAGE1D,
	OPCODE_TEX_IMAGE2D,
	OPCODE_TEX_IMAGE3D,
	OPCODE_TEX_SUB_IMAGE1D,
	OPCODE_TEX_SUB_IMAGE2D,
        OPCODE_TRANSLATE,
        OPCODE_VERTEX2,
        OPCODE_VERTEX3,
        OPCODE_VERTEX4,
	OPCODE_VIEWPORT,
	/* The following two are meta instructions */
	OPCODE_CONTINUE,
	OPCODE_END_OF_LIST
} OpCode;


/*
 * Each instruction in the display list is stored as a sequence of
 * contiguous nodes in memory.
 * Each node is the union of a variety of datatypes.
 */
union node {
	OpCode		opcode;
	GLboolean	b;
	GLbitfield	bf;
	GLubyte		ub;
	GLshort		s;
	GLushort	us;
	GLint		i;
	GLuint		ui;
	GLenum		e;
	GLfloat		f;
	GLvoid		*data;
	void		*next;	/* If prev node's opcode==OPCODE_CONTINUE */
};



/* Number of nodes of storage needed for each instruction: */
static GLuint InstSize[ OPCODE_END_OF_LIST+1 ];



/**********************************************************************/
/*****                           Private                          *****/
/**********************************************************************/


/*
 * Allocate space for a display list instruction.
 * Input:  opcode - type of instruction
 *         argcount - number of arguments following the instruction
 * Return: pointer to first node in the instruction
 */
static Node *alloc_instruction( OpCode opcode, GLint argcount )
{
   Node *n, *newblock;
   GLuint count = InstSize[opcode];

   assert( (GLint) count == argcount+1 );

   if (gCCPriv->CurrentPos + count + 2 > BLOCK_SIZE) {
      /* This block is full.  Allocate a new block and chain to it */
      n = gCCPriv->CurrentBlock + gCCPriv->CurrentPos;
      n[0].opcode = OPCODE_CONTINUE;
      newblock = (Node *) malloc( sizeof(Node) * BLOCK_SIZE );
      if (!newblock) {
         gamma_error( GL_OUT_OF_MEMORY, "Building display list" );
         return NULL;
      }
      n[1].next = (Node *) newblock;
      gCCPriv->CurrentBlock = newblock;
      gCCPriv->CurrentPos = 0;
   }

   n = gCCPriv->CurrentBlock + gCCPriv->CurrentPos;
   gCCPriv->CurrentPos += count;

   n[0].opcode = opcode;

   return n;
}



/*
 * Make an empty display list.  This is used by glGenLists() to
 * reserver display list IDs.
 */
static Node *make_empty_list( void )
{
   Node *n = (Node *) malloc( sizeof(Node) );
   n[0].opcode = OPCODE_END_OF_LIST;
   return n;
}



/*
 * Destroy all nodes in a display list.
 * Input:  list - display list number
 */
void gamma_destroy_list( GLuint list )
{
   Node *n, *block;
   GLboolean done;

   block = (Node *) _mesa_HashLookup(gCCPriv->DisplayList, list);
   n = block;

   done = block ? GL_FALSE : GL_TRUE;
   while (!done) {
      switch (n[0].opcode) {
	 /* special cases first */
	 case OPCODE_MAP1:
	    free( n[6].data );
	    n += InstSize[n[0].opcode];
	    break;
	 case OPCODE_MAP2:
	    free( n[10].data );
	    n += InstSize[n[0].opcode];
	    break;
	 case OPCODE_DRAW_PIXELS:
	    free( n[5].data );
	    n += InstSize[n[0].opcode];
	    break;
	 case OPCODE_BITMAP:
	    free( n[7].data );
	    n += InstSize[n[0].opcode];
	    break;
         case OPCODE_POLYGON_STIPPLE:
            free( n[1].data );
	    n += InstSize[n[0].opcode];
            break;
	 case OPCODE_TEX_IMAGE1D:
            free( n[8].data );
            n += InstSize[n[0].opcode];
	    break;
	 case OPCODE_TEX_IMAGE2D:
            free( n[9].data );
            n += InstSize[n[0].opcode];
	    break;
         case OPCODE_TEX_SUB_IMAGE1D:
            free( n[7].data );
            n += InstSize[n[0].opcode];
            break;
         case OPCODE_TEX_SUB_IMAGE2D:
            free( n[9].data );
            n += InstSize[n[0].opcode];
            break;
	 case OPCODE_CONTINUE:
	    n = (Node *) n[1].next;
	    free( block );
	    block = n;
	    break;
	 case OPCODE_END_OF_LIST:
	    free( block );
	    done = GL_TRUE;
	    break;
	 default:
	    /* Most frequent case */
	    n += InstSize[n[0].opcode];
	    break;
      }
   }

   _mesa_HashRemove(gCCPriv->DisplayList, list);
}



/*
 * Translate the nth element of list from type to GLuint.
 */
static GLuint translate_id( GLsizei n, GLenum type, const GLvoid *list )
{
   GLbyte *bptr;
   GLubyte *ubptr;
   GLshort *sptr;
   GLushort *usptr;
   GLint *iptr;
   GLuint *uiptr;
   GLfloat *fptr;

   switch (type) {
      case GL_BYTE:
         bptr = (GLbyte *) list;
         return (GLuint) *(bptr+n);
      case GL_UNSIGNED_BYTE:
         ubptr = (GLubyte *) list;
         return (GLuint) *(ubptr+n);
      case GL_SHORT:
         sptr = (GLshort *) list;
         return (GLuint) *(sptr+n);
      case GL_UNSIGNED_SHORT:
         usptr = (GLushort *) list;
         return (GLuint) *(usptr+n);
      case GL_INT:
         iptr = (GLint *) list;
         return (GLuint) *(iptr+n);
      case GL_UNSIGNED_INT:
         uiptr = (GLuint *) list;
         return (GLuint) *(uiptr+n);
      case GL_FLOAT:
         fptr = (GLfloat *) list;
         return (GLuint) *(fptr+n);
      case GL_2_BYTES:
         ubptr = ((GLubyte *) list) + 2*n;
         return (GLuint) *ubptr * 256 + (GLuint) *(ubptr+1);
      case GL_3_BYTES:
         ubptr = ((GLubyte *) list) + 3*n;
         return (GLuint) *ubptr * 65536
              + (GLuint) *(ubptr+1) * 256
              + (GLuint) *(ubptr+2);
      case GL_4_BYTES:
         ubptr = ((GLubyte *) list) + 4*n;
         return (GLuint) *ubptr * 16777216
              + (GLuint) *(ubptr+1) * 65536
              + (GLuint) *(ubptr+2) * 256
              + (GLuint) *(ubptr+3);
      default:
         return 0;
   }
}




/**********************************************************************/
/*****                        Public                              *****/
/**********************************************************************/

void gamma_init_lists( void )
{
   static int init_flag = 0;

   if (init_flag==0) {
      InstSize[OPCODE_ACCUM] = 3;
      InstSize[OPCODE_ALPHA_FUNC] = 3;
      InstSize[OPCODE_BEGIN] = 2;
      InstSize[OPCODE_BIND_TEXTURE] = 3;
      InstSize[OPCODE_BITMAP] = 8;
      InstSize[OPCODE_BLEND_FUNC] = 3;
      InstSize[OPCODE_CALL_LIST] = 2;
      InstSize[OPCODE_CALL_LIST_OFFSET] = 2;
      InstSize[OPCODE_CLEAR] = 2;
      InstSize[OPCODE_CLEAR_ACCUM] = 5;
      InstSize[OPCODE_CLEAR_COLOR] = 5;
      InstSize[OPCODE_CLEAR_DEPTH] = 2;
      InstSize[OPCODE_CLEAR_INDEX] = 2;
      InstSize[OPCODE_CLEAR_STENCIL] = 2;
      InstSize[OPCODE_CLIP_PLANE] = 6;
      InstSize[OPCODE_COLOR_3F] = 4;
      InstSize[OPCODE_COLOR_4F] = 5;
      InstSize[OPCODE_COLOR_4UB] = 5;
      InstSize[OPCODE_COLOR_MASK] = 5;
      InstSize[OPCODE_COLOR_MATERIAL] = 3;
      InstSize[OPCODE_COPY_PIXELS] = 6;
      InstSize[OPCODE_COPY_TEX_IMAGE1D] = 8;
      InstSize[OPCODE_COPY_TEX_IMAGE2D] = 9;
      InstSize[OPCODE_COPY_TEX_SUB_IMAGE1D] = 7;
      InstSize[OPCODE_COPY_TEX_SUB_IMAGE2D] = 9;
      InstSize[OPCODE_CULL_FACE] = 2;
      InstSize[OPCODE_DEPTH_FUNC] = 2;
      InstSize[OPCODE_DEPTH_MASK] = 2;
      InstSize[OPCODE_DEPTH_RANGE] = 3;
      InstSize[OPCODE_DISABLE] = 2;
      InstSize[OPCODE_DRAW_BUFFER] = 2;
      InstSize[OPCODE_DRAW_PIXELS] = 2;
      InstSize[OPCODE_ENABLE] = 2;
      InstSize[OPCODE_EDGE_FLAG] = 2;
      InstSize[OPCODE_END] = 1;
      InstSize[OPCODE_EVALCOORD1] = 2;
      InstSize[OPCODE_EVALCOORD2] = 3;
      InstSize[OPCODE_EVALMESH1] = 4;
      InstSize[OPCODE_EVALMESH2] = 6;
      InstSize[OPCODE_EVALPOINT1] = 2;
      InstSize[OPCODE_EVALPOINT2] = 3;
      InstSize[OPCODE_FOG] = 6;
      InstSize[OPCODE_FRONT_FACE] = 2;
      InstSize[OPCODE_FRUSTUM] = 7;
      InstSize[OPCODE_HINT] = 3;
      InstSize[OPCODE_INDEX] = 2;
      InstSize[OPCODE_INDEX_MASK] = 2;
      InstSize[OPCODE_INIT_NAMES] = 1;
      InstSize[OPCODE_LIGHT] = 7;
      InstSize[OPCODE_LIGHT_MODEL] = 6;
      InstSize[OPCODE_LINE_STIPPLE] = 3;
      InstSize[OPCODE_LINE_WIDTH] = 2;
      InstSize[OPCODE_LIST_BASE] = 2;
      InstSize[OPCODE_LOAD_IDENTITY] = 1;
      InstSize[OPCODE_LOAD_MATRIX] = 17;
      InstSize[OPCODE_LOAD_NAME] = 2;
      InstSize[OPCODE_LOGIC_OP] = 2;
      InstSize[OPCODE_MAP1] = 7;
      InstSize[OPCODE_MAP2] = 11;
      InstSize[OPCODE_MAPGRID1] = 4;
      InstSize[OPCODE_MAPGRID2] = 7;
      InstSize[OPCODE_MATERIAL] = 7;
      InstSize[OPCODE_MATRIX_MODE] = 2;
      InstSize[OPCODE_MULT_MATRIX] = 17;
      InstSize[OPCODE_NORMAL] = 4;
      InstSize[OPCODE_ORTHO] = 7;
      InstSize[OPCODE_PASSTHROUGH] = 2;
      InstSize[OPCODE_PIXEL_MAP] = 4;
      InstSize[OPCODE_PIXEL_TRANSFER] = 3;
      InstSize[OPCODE_PIXEL_ZOOM] = 3;
      InstSize[OPCODE_POINT_SIZE] = 2;
      InstSize[OPCODE_POLYGON_MODE] = 3;
      InstSize[OPCODE_POLYGON_STIPPLE] = 2;
      InstSize[OPCODE_POLYGON_OFFSET] = 3;
      InstSize[OPCODE_POP_ATTRIB] = 1;
      InstSize[OPCODE_POP_MATRIX] = 1;
      InstSize[OPCODE_POP_NAME] = 1;
      InstSize[OPCODE_PRIORITIZE_TEXTURE] = 3;
      InstSize[OPCODE_PUSH_ATTRIB] = 2;
      InstSize[OPCODE_PUSH_MATRIX] = 1;
      InstSize[OPCODE_PUSH_NAME] = 2;
      InstSize[OPCODE_RASTER_POS] = 5;
      InstSize[OPCODE_RECTF] = 5;
      InstSize[OPCODE_READ_BUFFER] = 2;
      InstSize[OPCODE_SCALE] = 4;
      InstSize[OPCODE_SCISSOR] = 5;
      InstSize[OPCODE_STENCIL_FUNC] = 4;
      InstSize[OPCODE_STENCIL_MASK] = 2;
      InstSize[OPCODE_STENCIL_OP] = 4;
      InstSize[OPCODE_SHADE_MODEL] = 2;
      InstSize[OPCODE_TEXCOORD2] = 3;
      InstSize[OPCODE_TEXCOORD4] = 5;
      InstSize[OPCODE_TEXENV] = 7;
      InstSize[OPCODE_TEXGEN] = 7;
      InstSize[OPCODE_TEXPARAMETER] = 7;
      InstSize[OPCODE_TEX_IMAGE1D] = 9;
      InstSize[OPCODE_TEX_IMAGE2D] = 10;
      InstSize[OPCODE_TEX_IMAGE3D] = 11;
      InstSize[OPCODE_TEX_SUB_IMAGE1D] = 8;
      InstSize[OPCODE_TEX_SUB_IMAGE2D] = 10;
      InstSize[OPCODE_TRANSLATE] = 4;
      InstSize[OPCODE_VERTEX2] = 3;
      InstSize[OPCODE_VERTEX3] = 4;
      InstSize[OPCODE_VERTEX4] = 5;
      InstSize[OPCODE_VIEWPORT] = 5;
      InstSize[OPCODE_CONTINUE] = 2;
      InstSize[OPCODE_END_OF_LIST] = 1;
   }
   init_flag = 1;
}


/*
 * Display List compilation functions
 */


void gl_save_Accum( GLenum op, GLfloat value )
{
   Node *n = alloc_instruction( OPCODE_ACCUM, 2 );
   if (n) {
      n[1].e = op;
      n[2].f = value;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Accum( op, value );
   }
}


void gl_save_AlphaFunc( GLenum func, GLclampf ref )
{
   Node *n = alloc_instruction( OPCODE_ALPHA_FUNC, 2 );
   if (n) {
      n[1].e = func;
      n[2].f = (GLfloat) ref;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_AlphaFunc( func, ref );
   }
}

void gl_save_ArrayElement( GLint i )
{
   if (gCCPriv->Array.NormalEnabled) {
      GLbyte *p = (GLbyte*) gCCPriv->Array.NormalPtr
                  + i * gCCPriv->Array.NormalStrideB;
      switch (gCCPriv->Array.NormalType) {
         case GL_BYTE:
            (*gCCPriv->API->Normal3bv)( (GLbyte*) p );
            break;
         case GL_SHORT:
            (*gCCPriv->API->Normal3sv)( (GLshort*) p );
            break;
         case GL_INT:
            (*gCCPriv->API->Normal3iv)( (GLint*) p );
            break;
         case GL_FLOAT:
            (*gCCPriv->API->Normal3fv)( (GLfloat*) p );
            break;
         case GL_DOUBLE:
            (*gCCPriv->API->Normal3dv)( (GLdouble*) p );
            break;
         default:
#if 0
            gl_problem("Bad normal type in gl_save_ArrayElement");
#endif
            return;
      }
   }

   if (gCCPriv->Array.ColorEnabled) {
      GLbyte *p = (GLbyte*) gCCPriv->Array.ColorPtr + i * gCCPriv->Array.ColorStrideB;
      switch (gCCPriv->Array.ColorType) {
         case GL_BYTE:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3bv)( (GLbyte*) p );   break;
               case 4:   (*gCCPriv->API->Color4bv)( (GLbyte*) p );   break;
            }
            break;
         case GL_UNSIGNED_BYTE:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3ubv)( (GLubyte*) p );   break;
               case 4:   (*gCCPriv->API->Color4ubv)( (GLubyte*) p );   break;
            }
            break;
         case GL_SHORT:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3sv)( (GLshort*) p );   break;
               case 4:   (*gCCPriv->API->Color4sv)( (GLshort*) p );   break;
            }
            break;
         case GL_UNSIGNED_SHORT:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3usv)( (GLushort*) p );   break;
               case 4:   (*gCCPriv->API->Color4usv)( (GLushort*) p );   break;
            }
            break;
         case GL_INT:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3iv)( (GLint*) p );   break;
               case 4:   (*gCCPriv->API->Color4iv)( (GLint*) p );   break;
            }
            break;
         case GL_UNSIGNED_INT:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3uiv)( (GLuint*) p );   break;
               case 4:   (*gCCPriv->API->Color4uiv)( (GLuint*) p );   break;
            }
            break;
         case GL_FLOAT:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3fv)( (GLfloat*) p );   break;
               case 4:   (*gCCPriv->API->Color4fv)( (GLfloat*) p );   break;
            }
            break;
         case GL_DOUBLE:
            switch (gCCPriv->Array.ColorSize) {
               case 3:   (*gCCPriv->API->Color3dv)( (GLdouble*) p );   break;
               case 4:   (*gCCPriv->API->Color4dv)( (GLdouble*) p );   break;
            }
            break;
         default:
#if 0
            gl_problem("Bad color type in gl_save_ArrayElement");
#endif
            return;
      }
   }

   if (gCCPriv->Array.IndexEnabled) {
      GLbyte *p = (GLbyte*) gCCPriv->Array.IndexPtr + i * gCCPriv->Array.IndexStrideB;
      switch (gCCPriv->Array.IndexType) {
         case GL_SHORT:
            (*gCCPriv->API->Indexsv)( (GLshort*) p );
            break;
         case GL_INT:
            (*gCCPriv->API->Indexiv)( (GLint*) p );
            break;
         case GL_FLOAT:
            (*gCCPriv->API->Indexfv)( (GLfloat*) p );
            break;
         case GL_DOUBLE:
            (*gCCPriv->API->Indexdv)( (GLdouble*) p );
            break;
         default:
#if 0
            gl_problem("Bad index type in gl_save_ArrayElement");
#endif
            return;
      }
   }

   if (gCCPriv->Array.EdgeFlagEnabled) {
      GLbyte *b = (GLbyte*) gCCPriv->Array.EdgeFlagPtr + i * gCCPriv->Array.EdgeFlagStrideB;
      (*gCCPriv->API->EdgeFlagv)( (GLboolean*) b );
   }

   if (gCCPriv->Array.VertexEnabled) {
      GLbyte *b = (GLbyte*) gCCPriv->Array.VertexPtr
                  + i * gCCPriv->Array.VertexStrideB;
      switch (gCCPriv->Array.VertexType) {
         case GL_SHORT:
            switch (gCCPriv->Array.VertexSize) {
               case 2:   (*gCCPriv->API->Vertex2sv)( (GLshort*) b );   break;
               case 3:   (*gCCPriv->API->Vertex3sv)( (GLshort*) b );   break;
               case 4:   (*gCCPriv->API->Vertex4sv)( (GLshort*) b );   break;
            }
            break;
         case GL_INT:
            switch (gCCPriv->Array.VertexSize) {
               case 2:   (*gCCPriv->API->Vertex2iv)( (GLint*) b );   break;
               case 3:   (*gCCPriv->API->Vertex3iv)( (GLint*) b );   break;
               case 4:   (*gCCPriv->API->Vertex4iv)( (GLint*) b );   break;
            }
            break;
         case GL_FLOAT:
            switch (gCCPriv->Array.VertexSize) {
               case 2:   (*gCCPriv->API->Vertex2fv)( (GLfloat*) b );   break;
               case 3:   (*gCCPriv->API->Vertex3fv)( (GLfloat*) b );   break;
               case 4:   (*gCCPriv->API->Vertex4fv)( (GLfloat*) b );   break;
            }
            break;
         case GL_DOUBLE:
            switch (gCCPriv->Array.VertexSize) {
               case 2:   (*gCCPriv->API->Vertex2dv)( (GLdouble*) b );   break;
               case 3:   (*gCCPriv->API->Vertex3dv)( (GLdouble*) b );   break;
               case 4:   (*gCCPriv->API->Vertex4dv)( (GLdouble*) b );   break;
            }
            break;
         default:
#if 0
            gl_problem("Bad vertex type in gl_save_ArrayElement");
#endif
            return;
      }
   }
}


void gl_save_Begin( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_BEGIN, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Begin( mode );
   }
}


void gl_save_BindTexture( GLenum target, GLuint texture )
{
   Node *n = alloc_instruction( OPCODE_BIND_TEXTURE, 2 );
   if (n) {
      n[1].e = target;
      n[2].ui = texture;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_BindTexture( target, texture );
   }
}


void gl_save_Bitmap( 
                     GLsizei width, GLsizei height,
		     GLfloat xorig, GLfloat yorig,
		     GLfloat xmove, GLfloat ymove,
		     const GLubyte *bitmap )
{
   GLvoid *image = _mesa_unpack_bitmap(width, height, bitmap, &gCCPriv->Unpack);
   Node *n = alloc_instruction( OPCODE_BITMAP, 7 );
   if (n) {
      n[1].i = (GLint) width;
      n[2].i = (GLint) height;
      n[3].f = xorig;
      n[4].f = yorig;
      n[5].f = xmove;
      n[6].f = ymove;
      n[7].data = image;
   } else
   if (image) {
      free(image);
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Bitmap( width, height,
                    xorig, yorig, xmove, ymove, bitmap );
   }
}


void gl_save_BlendFunc( GLenum sfactor, GLenum dfactor )
{
   Node *n = alloc_instruction( OPCODE_BLEND_FUNC, 2 );
   if (n) {
      n[1].e = sfactor;
      n[2].e = dfactor;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_BlendFunc( sfactor, dfactor );
   }
}

void gl_save_CallList( GLuint list )
{
   Node *n = alloc_instruction( OPCODE_CALL_LIST, 1 );
   if (n) {
      n[1].ui = list;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CallList( list );
   }
}


void gl_save_CallLists( 
                        GLsizei n, GLenum type, const GLvoid *lists )
{
   GLint i;

   for (i=0;i<n;i++) {
      GLuint list = translate_id( i, type, lists );
      Node *n = alloc_instruction( OPCODE_CALL_LIST_OFFSET, 1 );
      if (n) {
         n[1].ui = list;
      }
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CallLists( n, type, lists );
   }
}


void gl_save_Clear( GLbitfield mask )
{
   Node *n = alloc_instruction( OPCODE_CLEAR, 1 );
   if (n) {
      n[1].bf = mask;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Clear( mask );
   }
}


void gl_save_ClearAccum( GLfloat red, GLfloat green,
			 GLfloat blue, GLfloat alpha )
{
   Node *n = alloc_instruction( OPCODE_CLEAR_ACCUM, 4 );
   if (n) {
      n[1].f = red;
      n[2].f = green;
      n[3].f = blue;
      n[4].f = alpha;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ClearAccum( red, green, blue, alpha );
   }
}


void gl_save_ClearColor( GLclampf red, GLclampf green,
			 GLclampf blue, GLclampf alpha )
{
   Node *n = alloc_instruction( OPCODE_CLEAR_COLOR, 4 );
   if (n) {
      n[1].f = red;
      n[2].f = green;
      n[3].f = blue;
      n[4].f = alpha;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ClearColor( red, green, blue, alpha );
   }
}


void gl_save_ClearDepth( GLclampd depth )
{
   Node *n = alloc_instruction( OPCODE_CLEAR_DEPTH, 1 );
   if (n) {
      n[1].f = (GLfloat) depth;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ClearDepth( depth );
   }
}


void gl_save_ClearIndex( GLfloat c )
{
   Node *n = alloc_instruction( OPCODE_CLEAR_INDEX, 1 );
   if (n) {
      n[1].f = c;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ClearIndex( c );
   }
}


void gl_save_ClearStencil( GLint s )
{
   Node *n = alloc_instruction( OPCODE_CLEAR_STENCIL, 1 );
   if (n) {
      n[1].i = s;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ClearStencil( s );
   }
}


void gl_save_ClipPlane( GLenum plane, const GLdouble *equ )
{
   Node *n = alloc_instruction( OPCODE_CLIP_PLANE, 5 );
   if (n) {
      n[1].e = plane;
      n[2].f = equ[0];
      n[3].f = equ[1];
      n[4].f = equ[2];
      n[5].f = equ[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ClipPlane( plane, equ );
   }
}


void gl_save_Color3f( GLfloat r, GLfloat g, GLfloat b )
{
   Node *n = alloc_instruction( OPCODE_COLOR_3F, 3 );
   if (n) {
      n[1].f = r;
      n[2].f = g;
      n[3].f = b;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Color3f( r, g, b );
   }
}


void gl_save_Color3fv( const GLfloat *c )
{
   Node *n = alloc_instruction( OPCODE_COLOR_3F, 3 );
   if (n) {
      n[1].f = c[0];
      n[2].f = c[1];
      n[3].f = c[2];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Color3fv( c );
   }
}


void gl_save_Color4f( GLfloat r, GLfloat g,
                                      GLfloat b, GLfloat a )
{
   Node *n = alloc_instruction( OPCODE_COLOR_4F, 4 );
   if (n) {
      n[1].f = r;
      n[2].f = g;
      n[3].f = b;
      n[4].f = a;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Color4f( r, g, b, a );
   }
}


void gl_save_Color4fv( const GLfloat *c )
{
   Node *n = alloc_instruction( OPCODE_COLOR_4F, 4 );
   if (n) {
      n[1].f = c[0];
      n[2].f = c[1];
      n[3].f = c[2];
      n[4].f = c[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Color4fv( c );
   }
}


void gl_save_Color4ub( GLubyte r, GLubyte g,
                                       GLubyte b, GLubyte a )
{
   Node *n = alloc_instruction( OPCODE_COLOR_4UB, 4 );
   if (n) {
      n[1].ub = r;
      n[2].ub = g;
      n[3].ub = b;
      n[4].ub = a;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Color4ub( r, g, b, a );
   }
}


void gl_save_Color4ubv( const GLubyte *c )
{
   Node *n = alloc_instruction( OPCODE_COLOR_4UB, 4 );
   if (n) {
      n[1].ub = c[0];
      n[2].ub = c[1];
      n[3].ub = c[2];
      n[4].ub = c[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Color4ubv( c );
   }
}


void gl_save_ColorMask( GLboolean red, GLboolean green,
                        GLboolean blue, GLboolean alpha )
{
   Node *n = alloc_instruction( OPCODE_COLOR_MASK, 4 );
   if (n) {
      n[1].b = red;
      n[2].b = green;
      n[3].b = blue;
      n[4].b = alpha;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ColorMask( red, green, blue, alpha );
   }
}


void gl_save_ColorMaterial( GLenum face, GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_COLOR_MATERIAL, 2 );
   if (n) {
      n[1].e = face;
      n[2].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ColorMaterial( face, mode );
   }
}


void gl_save_CopyPixels( GLint x, GLint y,
			 GLsizei width, GLsizei height, GLenum type )
{
   Node *n = alloc_instruction( OPCODE_COPY_PIXELS, 5 );
   if (n) {
      n[1].i = x;
      n[2].i = y;
      n[3].i = (GLint) width;
      n[4].i = (GLint) height;
      n[5].e = type;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CopyPixels( x, y, width, height, type );
   }
}



void gl_save_CopyTexImage1D( 
                             GLenum target, GLint level,
                             GLenum internalformat,
                             GLint x, GLint y, GLsizei width,
                             GLint border )
{
   Node *n = alloc_instruction( OPCODE_COPY_TEX_IMAGE1D, 7 );
   if (n) {
      n[1].e = target;
      n[2].i = level;
      n[3].e = internalformat;
      n[4].i = x;
      n[5].i = y;
      n[6].i = width;
      n[7].i = border;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CopyTexImage1D( target, level, internalformat,
                            x, y, width, border );
   }
}


void gl_save_CopyTexImage2D( 
                             GLenum target, GLint level,
                             GLenum internalformat,
                             GLint x, GLint y, GLsizei width,
                             GLsizei height, GLint border )
{
   Node *n = alloc_instruction( OPCODE_COPY_TEX_IMAGE2D, 8 );
   if (n) {
      n[1].e = target;
      n[2].i = level;
      n[3].e = internalformat;
      n[4].i = x;
      n[5].i = y;
      n[6].i = width;
      n[7].i = height;
      n[8].i = border;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CopyTexImage2D( target, level, internalformat,
                            x, y, width, height, border );
   }
}



void gl_save_CopyTexSubImage1D( 
                                GLenum target, GLint level,
                                GLint xoffset, GLint x, GLint y,
                                GLsizei width )
{
   Node *n = alloc_instruction( OPCODE_COPY_TEX_SUB_IMAGE1D, 6 );
   if (n) {
      n[1].e = target;
      n[2].i = level;
      n[3].i = xoffset;
      n[4].i = x;
      n[5].i = y;
      n[6].i = width;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CopyTexSubImage1D( target, level, xoffset, x, y, width );
   }
}


void gl_save_CopyTexSubImage2D( 
                                GLenum target, GLint level,
                                GLint xoffset, GLint yoffset,
                                GLint x, GLint y,
                                GLsizei width, GLint height )
{
   Node *n = alloc_instruction( OPCODE_COPY_TEX_SUB_IMAGE2D, 8 );
   if (n) {
      n[1].e = target;
      n[2].i = level;
      n[3].i = xoffset;
      n[4].i = yoffset;
      n[5].i = x;
      n[6].i = y;
      n[7].i = width;
      n[8].i = height;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CopyTexSubImage2D( target, level, xoffset, yoffset,
                               x, y, width, height );
   }
}


void gl_save_CullFace( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_CULL_FACE, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_CullFace( mode );
   }
}


void gl_save_DepthFunc( GLenum func )
{
   Node *n = alloc_instruction( OPCODE_DEPTH_FUNC, 1 );
   if (n) {
      n[1].e = func;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_DepthFunc( func );
   }
}


void gl_save_DepthMask( GLboolean mask )
{
   Node *n = alloc_instruction( OPCODE_DEPTH_MASK, 1 );
   if (n) {
      n[1].b = mask;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_DepthMask( mask );
   }
}


void gl_save_DepthRange( GLclampd nearval, GLclampd farval )
{
   Node *n = alloc_instruction( OPCODE_DEPTH_RANGE, 2 );
   if (n) {
      n[1].f = (GLfloat) nearval;
      n[2].f = (GLfloat) farval;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_DepthRange( nearval, farval );
   }
}


void gl_save_Disable( GLenum cap )
{
   Node *n = alloc_instruction( OPCODE_DISABLE, 1 );
   if (n) {
      n[1].e = cap;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Disable( cap );
   }
}

void gl_save_DrawArrays( 
                         GLenum mode, GLint first, GLsizei count )
{
   GLint i;

#if 0
   if (INSIDE_BEGIN_END(ctx)) {
      gamma_error( GL_INVALID_OPERATION, "glDrawArrays" );
      return;
   }
#endif
   if (count<0) {
      gamma_error( GL_INVALID_VALUE, "glDrawArrays(count)" );
      return;
   }
   switch (mode) {
      case GL_POINTS:
      case GL_LINES:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_QUADS:
      case GL_QUAD_STRIP:
      case GL_POLYGON:
         /* OK */
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glDrawArrays(mode)" );
         return;
   }

   /* Note: this will do compile AND execute if needed */
   gl_save_Begin( mode );
   for (i=0;i<count;i++) {
      gl_save_ArrayElement( first+i );
   }
   gl_save_End( );
}


void gl_save_DrawBuffer( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_DRAW_BUFFER, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_DrawBuffer( mode );
   }
}


void gl_save_DrawElements(
                           GLenum mode, GLsizei count,
                           GLenum type, const GLvoid *indices )
{
   switch (type) {
      case GL_UNSIGNED_BYTE:
         {
            GLubyte *ub_indices = (GLubyte *) indices;
            GLint i;
            gl_save_Begin( mode );
            for (i=0;i<count;i++) {
               gl_save_ArrayElement( (GLint) ub_indices[i] );
            }
            gl_save_End( );
         }
         break;
      case GL_UNSIGNED_SHORT:
         {
            GLushort *us_indices = (GLushort *) indices;
            GLint i;
            gl_save_Begin( mode );
            for (i=0;i<count;i++) {
               gl_save_ArrayElement( (GLint) us_indices[i] );
            }
            gl_save_End( );
         }
         break;
      case GL_UNSIGNED_INT:
         {
            GLuint *ui_indices = (GLuint *) indices;
            GLint i;
            gl_save_Begin( mode );
            for (i=0;i<count;i++) {
               gl_save_ArrayElement( (GLint) ui_indices[i] );
            }
            gl_save_End( );
         }
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glDrawElements(type)" );
         return;
   }
}


void gl_save_DrawPixels( GLsizei width, GLsizei height,
                         GLenum format, GLenum type,
                         const GLvoid *pixels )
{
   GLvoid *image = _mesa_unpack_image(width, height, 1, format, type,
                                      pixels, &gCCPriv->Unpack);
   Node *n = alloc_instruction( OPCODE_DRAW_PIXELS, 5 );
   if (n) {
      n[1].i = width;
      n[2].i = height;
      n[3].e = format;
      n[4].e = type;
      n[5].data = image;
   } else
   if (image) {
      free(image);
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_DrawPixels( width, height, format, type, pixels );
   }
}


void gl_save_EdgeFlag( GLboolean flag )
{
   Node *n = alloc_instruction( OPCODE_EDGE_FLAG, 1 );
   if (n) {
      n[1].b = flag;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EdgeFlag( flag );
   }
}


void gl_save_Enable( GLenum cap )
{
   Node *n = alloc_instruction( OPCODE_ENABLE, 1 );
   if (n) {
      n[1].e = cap;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Enable( cap );
   }
}


void gl_save_End( void )
{
   (void) alloc_instruction( OPCODE_END, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_End( );
   }
}


void gl_save_EvalCoord1f( GLfloat u )
{
   Node *n = alloc_instruction( OPCODE_EVALCOORD1, 1 );
   if (n) {
      n[1].f = u;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EvalCoord1f( u );
   }
}


void gl_save_EvalCoord2f( GLfloat u, GLfloat v )
{
   Node *n = alloc_instruction( OPCODE_EVALCOORD2, 2 );
   if (n) {
      n[1].f = u;
      n[2].f = v;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EvalCoord2f( u, v );
   }
}


void gl_save_EvalMesh1( 
                        GLenum mode, GLint i1, GLint i2 )
{
   Node *n = alloc_instruction( OPCODE_EVALMESH1, 3 );
   if (n) {
      n[1].e = mode;
      n[2].i = i1;
      n[3].i = i2;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EvalMesh1( mode, i1, i2 );
   }
}


void gl_save_EvalMesh2(  
                        GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
   Node *n = alloc_instruction( OPCODE_EVALMESH2, 5 );
   if (n) {
      n[1].e = mode;
      n[2].i = i1;
      n[3].i = i2;
      n[4].i = j1;
      n[5].i = j2;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EvalMesh2( mode, i1, i2, j1, j2 );
   }
}


void gl_save_EvalPoint1( GLint i )
{
   Node *n = alloc_instruction( OPCODE_EVALPOINT1, 1 );
   if (n) {
      n[1].i = i;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EvalPoint1( i );
   }
}


void gl_save_EvalPoint2( GLint i, GLint j )
{
   Node *n = alloc_instruction( OPCODE_EVALPOINT2, 2 );
   if (n) {
      n[1].i = i;
      n[2].i = j;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_EvalPoint2( i, j );
   }
}


void gl_save_Fogfv( GLenum pname, const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_FOG, 5 );
   if (n) {
      n[1].e = pname;
      n[2].f = params[0];
      n[3].f = params[1];
      n[4].f = params[2];
      n[5].f = params[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Fogfv( pname, params );
   }
}


void gl_save_FrontFace( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_FRONT_FACE, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_FrontFace( mode );
   }
}


void gl_save_Frustum( GLdouble left, GLdouble right,
                      GLdouble bottom, GLdouble top,
                      GLdouble nearval, GLdouble farval )
{
   Node *n = alloc_instruction( OPCODE_FRUSTUM, 6 );
   if (n) {
      n[1].f = left;
      n[2].f = right;
      n[3].f = bottom;
      n[4].f = top;
      n[5].f = nearval;
      n[6].f = farval;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Frustum( left, right, bottom, top, nearval, farval );
   }
}


void gl_save_Hint( GLenum target, GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_HINT, 2 );
   if (n) {
      n[1].e = target;
      n[2].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Hint( target, mode );
   }
}


void gl_save_Indexi( GLint index )
{
   Node *n = alloc_instruction( OPCODE_INDEX, 1 );
   if (n) {
      n[1].i = index;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Indexi( index );
   }
}


void gl_save_Indexf( GLfloat index )
{
   Node *n = alloc_instruction( OPCODE_INDEX, 1 );
   if (n) {
      n[1].i = (GLint) index;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Indexf( index );
   }
}


void gl_save_IndexMask( GLuint mask )
{
   Node *n = alloc_instruction( OPCODE_INDEX_MASK, 1 );
   if (n) {
      n[1].ui = mask;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_IndexMask( mask );
   }
}


void gl_save_InitNames( void )
{
   (void) alloc_instruction( OPCODE_INIT_NAMES, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_InitNames( );
   }
}


void gl_save_Lightfv( GLenum light, GLenum pname,
                      const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_LIGHT, 6 );
   if (OPCODE_LIGHT) {
      GLint i, nParams;
      n[1].e = light;
      n[2].e = pname;
      switch (pname) {
         case GL_AMBIENT:
            nParams = 4;
            break;
         case GL_DIFFUSE:
            nParams = 4;
            break;
         case GL_SPECULAR:
            nParams = 4;
            break;
         case GL_POSITION:
            nParams = 4;
            break;
         case GL_SPOT_DIRECTION:
            nParams = 3;
            break;
         case GL_SPOT_EXPONENT:
            nParams = 1;
            break;
         case GL_SPOT_CUTOFF:
            nParams = 1;
            break;
         case GL_CONSTANT_ATTENUATION:
            nParams = 1;
            break;
         case GL_LINEAR_ATTENUATION:
            nParams = 1;
            break;
         case GL_QUADRATIC_ATTENUATION:
            nParams = 1;
            break;
         default:
            nParams = 0;
      }
      for (i = 0; i < nParams; i++) {
         n[3+i].f = params[i];
      }
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Lightfv( light, pname, params );
   }
}


void gl_save_LightModelfv( 
                           GLenum pname, const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_LIGHT_MODEL, 5 );
   if (n) {
      n[1].e = pname;
      n[2].f = params[0];
      n[3].f = params[1];
      n[4].f = params[2];
      n[5].f = params[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_LightModelfv( pname, params );
   }
}


void gl_save_LineStipple( GLint factor, GLushort pattern )
{
   Node *n = alloc_instruction( OPCODE_LINE_STIPPLE, 2 );
   if (n) {
      n[1].i = factor;
      n[2].us = pattern;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_LineStipple( factor, pattern );
   }
}


void gl_save_LineWidth( GLfloat width )
{
   Node *n = alloc_instruction( OPCODE_LINE_WIDTH, 1 );
   if (n) {
      n[1].f = width;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_LineWidth( width );
   }
}


void gl_save_ListBase( GLuint base )
{
   Node *n = alloc_instruction( OPCODE_LIST_BASE, 1 );
   if (n) {
      n[1].ui = base;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ListBase( base );
   }
}


void gl_save_LoadIdentity( void )
{
   (void) alloc_instruction( OPCODE_LOAD_IDENTITY, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_LoadIdentity( );
   }
}


void gl_save_LoadMatrixf( const GLfloat *m )
{
   Node *n = alloc_instruction( OPCODE_LOAD_MATRIX, 16 );
   if (n) {
      GLuint i;
      for (i=0;i<16;i++) {
	 n[1+i].f = m[i];
      }
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_LoadMatrixf( m );
   }
}


void gl_save_LoadName( GLuint name )
{
   Node *n = alloc_instruction( OPCODE_LOAD_NAME, 1 );
   if (n) {
      n[1].ui = name;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_LoadName( name );
   }
}


void gl_save_LogicOp( GLenum opcode )
{
   Node *n = alloc_instruction( OPCODE_LOGIC_OP, 1 );
   if (n) {
      n[1].e = opcode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_LogicOp( opcode );
   }
}


void gl_save_Map1f( 
                   GLenum target, GLfloat u1, GLfloat u2, GLint stride,
		   GLint order, const GLfloat *points )
{
   Node *n = alloc_instruction( OPCODE_MAP1, 6 );
   if (n) {
      GLfloat *pnts = gl_copy_map_points1f( target, stride, order, points );
      n[1].e = target;
      n[2].f = u1;
      n[3].f = u2;
      n[4].i = _mesa_evaluator_components(target);  /* stride */
      n[5].i = order;
      n[6].data = (void *) pnts;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Map1f( target, u1, u2, stride, order, points );
   }
}


void gl_save_Map2f( GLenum target,
                    GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                    GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
                    const GLfloat *points )
{
   Node *n = alloc_instruction( OPCODE_MAP2, 10 );
   if (n) {
      GLfloat *pnts = gl_copy_map_points2f( target, ustride, uorder,
                                            vstride, vorder, points );
      n[1].e = target;
      n[2].f = u1;
      n[3].f = u2;
      n[4].f = v1;
      n[5].f = v2;
      /* XXX verify these strides are correct */
      n[6].i = _mesa_evaluator_components(target) * vorder;  /*ustride*/
      n[7].i = _mesa_evaluator_components(target);           /*vstride*/
      n[8].i = uorder;
      n[9].i = vorder;
      n[10].data = (void *) pnts;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Map2f( target,
                        u1, u2, ustride, uorder,
                        v1, v2, vstride, vorder, points );
   }
}


void gl_save_MapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{
   Node *n = alloc_instruction( OPCODE_MAPGRID1, 3 );
   if (n) {
      n[1].i = un;
      n[2].f = u1;
      n[3].f = u2;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_MapGrid1f( un, u1, u2 );
   }
}


void gl_save_MapGrid2f( 
                        GLint un, GLfloat u1, GLfloat u2,
		        GLint vn, GLfloat v1, GLfloat v2 )
{
   Node *n = alloc_instruction( OPCODE_MAPGRID2, 6 );
   if (n) {
      n[1].i = un;
      n[2].f = u1;
      n[3].f = u2;
      n[4].i = vn;
      n[5].f = v1;
      n[6].f = v2;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_MapGrid2f( un, u1, u2, vn, v1, v2 );
   }
}


void gl_save_Materialfv(
                         GLenum face, GLenum pname, const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_MATERIAL, 6 );
   if (n) {
      n[1].e = face;
      n[2].e = pname;
      n[3].f = params[0];
      n[4].f = params[1];
      n[5].f = params[2];
      n[6].f = params[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Materialfv( face, pname, params );
   }
}


void gl_save_MatrixMode( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_MATRIX_MODE, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_MatrixMode( mode );
   }
}


void gl_save_MultMatrixf( const GLfloat *m )
{
   Node *n = alloc_instruction( OPCODE_MULT_MATRIX, 16 );
   if (n) {
      GLuint i;
      for (i=0;i<16;i++) {
	 n[1+i].f = m[i];
      }
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_MultMatrixf( m );
   }
}


void gl_save_NewList( GLuint list, GLenum mode )
{
   /* It's an error to call this function while building a display list */
   gamma_error( GL_INVALID_OPERATION, "glNewList" );
   (void) list;
   (void) mode;
}


void gl_save_Normal3fv( const GLfloat norm[3] )
{
   Node *n = alloc_instruction( OPCODE_NORMAL, 3 );
   if (n) {
      n[1].f = norm[0];
      n[2].f = norm[1];
      n[3].f = norm[2];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Normal3fv( norm );
   }
}


void gl_save_Normal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
   Node *n = alloc_instruction( OPCODE_NORMAL, 3 );
   if (n) {
      n[1].f = nx;
      n[2].f = ny;
      n[3].f = nz;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Normal3f( nx, ny, nz );
   }
}


void gl_save_Ortho( GLdouble left, GLdouble right,
                    GLdouble bottom, GLdouble top,
                    GLdouble nearval, GLdouble farval )
{
   Node *n = alloc_instruction( OPCODE_ORTHO, 6 );
   if (n) {
      n[1].f = left;
      n[2].f = right;
      n[3].f = bottom;
      n[4].f = top;
      n[5].f = nearval;
      n[6].f = farval;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Ortho( left, right, bottom, top, nearval, farval );
   }
}


void gl_save_PixelMapfv( 
                         GLenum map, GLint mapsize, const GLfloat *values )
{
   Node *n = alloc_instruction( OPCODE_PIXEL_MAP, 3 );
   if (n) {
      n[1].e = map;
      n[2].i = mapsize;
      n[3].data  = (void *) malloc( mapsize * sizeof(GLfloat) );
      MEMCPY( n[3].data, (void *) values, mapsize * sizeof(GLfloat) );
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PixelMapfv( map, mapsize, values );
   }
}


void gl_save_PixelTransferf( GLenum pname, GLfloat param )
{
   Node *n = alloc_instruction( OPCODE_PIXEL_TRANSFER, 2 );
   if (n) {
      n[1].e = pname;
      n[2].f = param;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PixelTransferf( pname, param );
   }
}


void gl_save_PixelZoom( GLfloat xfactor, GLfloat yfactor )
{
   Node *n = alloc_instruction( OPCODE_PIXEL_ZOOM, 2 );
   if (n) {
      n[1].f = xfactor;
      n[2].f = yfactor;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PixelZoom( xfactor, yfactor );
   }
}


void gl_save_PointSize( GLfloat size )
{
   Node *n = alloc_instruction( OPCODE_POINT_SIZE, 1 );
   if (n) {
      n[1].f = size;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PointSize( size );
   }
}


void gl_save_PolygonMode( GLenum face, GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_POLYGON_MODE, 2 );
   if (n) {
      n[1].e = face;
      n[2].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PolygonMode( face, mode );
   }
}


/*
 * Polygon stipple must have been upacked already!
 */
void gl_save_PolygonStipple( const GLubyte *pattern )
{
   Node *n = alloc_instruction( OPCODE_POLYGON_STIPPLE, 1 );
   if (n) {
      void *data;
      n[1].data = malloc( 32 * 4 );
      data = n[1].data;   /* This needed for Acorn compiler */
      MEMCPY( data, pattern, 32 * 4 );
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PolygonStipple( pattern );
   }
}


void gl_save_PolygonOffset( GLfloat factor, GLfloat units )
{
   Node *n = alloc_instruction( OPCODE_POLYGON_OFFSET, 2 );
   if (n) {
      n[1].f = factor;
      n[2].f = units;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PolygonOffset( factor, units );
   }
}


void gl_save_PopAttrib( void )
{
   (void) alloc_instruction( OPCODE_POP_ATTRIB, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_PopAttrib( );
   }
}


void gl_save_PopMatrix( void )
{
   (void) alloc_instruction( OPCODE_POP_MATRIX, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_PopMatrix( );
   }
}


void gl_save_PopName( void )
{
   (void) alloc_instruction( OPCODE_POP_NAME, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_PopName( );
   }
}


void gl_save_PrioritizeTextures( 
                                 GLsizei num, const GLuint *textures,
                                 const GLclampf *priorities )
{
   GLint i;

   for (i=0;i<num;i++) {
      Node *n = alloc_instruction( OPCODE_PRIORITIZE_TEXTURE, 2 );
      if (n) {
         n[1].ui = textures[i];
         n[2].f = priorities[i];
      }
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PrioritizeTextures( num, textures, priorities );
   }
}


void gl_save_PushAttrib( GLbitfield mask )
{
   Node *n = alloc_instruction( OPCODE_PUSH_ATTRIB, 1 );
   if (n) {
      n[1].bf = mask;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PushAttrib( mask );
   }
}


void gl_save_PushMatrix( void )
{
   (void) alloc_instruction( OPCODE_PUSH_MATRIX, 0 );
   if (gCCPriv->ExecuteFlag) {
      _gamma_PushMatrix( );
   }
}


void gl_save_PushName( GLuint name )
{
   Node *n = alloc_instruction( OPCODE_PUSH_NAME, 1 );
   if (n) {
      n[1].ui = name;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PushName( name );
   }
}


void gl_save_RasterPos4f( 
                          GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   Node *n = alloc_instruction( OPCODE_RASTER_POS, 4 );
   if (n) {
      n[1].f = x;
      n[2].f = y;
      n[3].f = z;
      n[4].f = w;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_RasterPos4f( x, y, z, w );
   }
}


void gl_save_PassThrough( GLfloat token )
{
   Node *n = alloc_instruction( OPCODE_PASSTHROUGH, 1 );
   if (n) {
      n[1].f = token;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_PassThrough( token );
   }
}


void gl_save_ReadBuffer( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_READ_BUFFER, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ReadBuffer( mode );
   }
}


void gl_save_Rectf( 
                    GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
   Node *n = alloc_instruction( OPCODE_RECTF, 4 );
   if (n) {
      n[1].f = x1;
      n[2].f = y1;
      n[3].f = x2;
      n[4].f = y2;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Rectf( x1, y1, x2, y2 );
   }
}


void gl_save_Rotatef( GLfloat angle,
                      GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat m[16];
   gl_rotation_matrix( angle, x, y, z, m );
   gl_save_MultMatrixf( m );  /* save and maybe execute */
}


void gl_save_Scalef( GLfloat x, GLfloat y, GLfloat z )
{
   Node *n = alloc_instruction( OPCODE_SCALE, 3 );
   if (n) {
      n[1].f = x;
      n[2].f = y;
      n[3].f = z;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Scalef( x, y, z );
   }
}


void gl_save_Scissor( 
                      GLint x, GLint y, GLsizei width, GLsizei height )
{
   Node *n = alloc_instruction( OPCODE_SCISSOR, 4 );
   if (n) {
      n[1].i = x;
      n[2].i = y;
      n[3].i = width;
      n[4].i = height;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Scissor( x, y, width, height );
   }
}


void gl_save_ShadeModel( GLenum mode )
{
   Node *n = alloc_instruction( OPCODE_SHADE_MODEL, 1 );
   if (n) {
      n[1].e = mode;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_ShadeModel( mode );
   }
}


void gl_save_StencilFunc( GLenum func, GLint ref, GLuint mask )
{
   Node *n = alloc_instruction( OPCODE_STENCIL_FUNC, 3 );
   if (n) {
      n[1].e = func;
      n[2].i = ref;
      n[3].ui = mask;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_StencilFunc( func, ref, mask );
   }
}


void gl_save_StencilMask( GLuint mask )
{
   Node *n = alloc_instruction( OPCODE_STENCIL_MASK, 1 );
   if (n) {
      n[1].ui = mask;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_StencilMask( mask );
   }
}


void gl_save_StencilOp( 
                        GLenum fail, GLenum zfail, GLenum zpass )
{
   Node *n = alloc_instruction( OPCODE_STENCIL_OP, 3 );
   if (n) {
      n[1].e = fail;
      n[2].e = zfail;
      n[3].e = zpass;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_StencilOp( fail, zfail, zpass );
   }
}


void gl_save_TexCoord2f( GLfloat s, GLfloat t )
{
   Node *n = alloc_instruction( OPCODE_TEXCOORD2, 2 );
   if (n) {
      n[1].f = s;
      n[2].f = t;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexCoord2f( s, t );
   }
}

void gl_save_TexCoord2fv( const GLfloat *v )
{
   Node *n = alloc_instruction( OPCODE_TEXCOORD2, 2 );
   if (n) {
      n[1].f = v[0];
      n[2].f = v[1];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexCoord2f( n[1].f, n[2].f );
   }
}

void gl_save_TexCoord3fv( const GLfloat *v )
{
   Node *n = alloc_instruction( OPCODE_TEXCOORD4, 4 );
   if (n) {
      n[1].f = v[0];
      n[2].f = v[1];
      n[1].f = v[2];
      n[2].f = 1.0f;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexCoord4f( n[1].f, n[2].f, n[3].f, n[4].f );
   }
}

void gl_save_TexCoord4f( GLfloat s, GLfloat t,
                                         GLfloat r, GLfloat q )
{
   Node *n = alloc_instruction( OPCODE_TEXCOORD4, 4 );
   if (n) {
      n[1].f = s;
      n[2].f = t;
      n[3].f = r;
      n[4].f = q;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexCoord4f( s, t, r, q );
   }
}


void gl_save_TexEnvfv(
                       GLenum target, GLenum pname, const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_TEXENV, 6 );
   if (n) {
      n[1].e = target;
      n[2].e = pname;
      n[3].f = params[0];
      n[4].f = params[1];
      n[5].f = params[2];
      n[6].f = params[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexEnvfv( target, pname, params );
   }
}


void gl_save_TexGenfv( 
                       GLenum coord, GLenum pname, const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_TEXGEN, 6 );
   if (n) {
      n[1].e = coord;
      n[2].e = pname;
      n[3].f = params[0];
      n[4].f = params[1];
      n[5].f = params[2];
      n[6].f = params[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexGenfv( coord, pname, params );
   }
}


void gl_save_TexParameterfv( GLenum target,
                             GLenum pname, const GLfloat *params )
{
   Node *n = alloc_instruction( OPCODE_TEXPARAMETER, 6 );
   if (n) {
      n[1].e = target;
      n[2].e = pname;
      n[3].f = params[0];
      n[4].f = params[1];
      n[5].f = params[2];
      n[6].f = params[3];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexParameterfv( target, pname, params );
   }
}


void gl_save_TexImage1D( GLenum target,
                         GLint level, GLint components,
			 GLsizei width, GLint border,
                         GLenum format, GLenum type,
			 const GLvoid *pixels )
{
   if (target == GL_PROXY_TEXTURE_1D) {
      /* don't compile, execute immediately */
      _gamma_TexImage1D( target, level, components, width,
                               border, format, type, pixels );
   }
   else {
      GLvoid *image = _mesa_unpack_image(width, 1, 1, format, type,
                                         pixels, &gCCPriv->Unpack);
      Node *n;
      n = alloc_instruction( OPCODE_TEX_IMAGE1D, 8 );
      if (n) {
         n[1].e = target;
         n[2].i = level;
         n[3].i = components;
         n[4].i = (GLint) width;
         n[5].i = border;
         n[6].e = format;
         n[7].e = type;
         n[8].data = image;
      }
      else if (image) {
         FREE(image);
      }
      if (gCCPriv->ExecuteFlag) {
         _gamma_TexImage1D( target, level, components, width,
                                  border, format, type, pixels );
      }
   }
}


void gl_save_TexImage2D( GLenum target,
                         GLint level, GLint components,
			 GLsizei width, GLsizei height, GLint border,
                         GLenum format, GLenum type,
			 const GLvoid *pixels )
{
   if (target == GL_PROXY_TEXTURE_2D) {
      /* don't compile, execute immediately */
      _gamma_TexImage2D( target, level, components, width,
                               height, border, format, type, pixels );
   }
   else {
      GLvoid *image = _mesa_unpack_image(width, height, 1, format, type,
                                         pixels, &gCCPriv->Unpack);
      Node *n;
      n = alloc_instruction( OPCODE_TEX_IMAGE2D, 9 );
      if (n) {
         n[1].e = target;
         n[2].i = level;
         n[3].i = components;
         n[4].i = (GLint) width;
         n[5].i = (GLint) height;
         n[6].i = border;
         n[7].e = format;
         n[8].e = type;
         n[9].data = image;
      }
      else if (image) {
         FREE(image);
      }
      if (gCCPriv->ExecuteFlag) {
         _gamma_TexImage2D( target, level, components, width,
                                  height, border, format, type, pixels );
      }
   }
}


void gl_save_TexSubImage1D( 
                            GLenum target, GLint level, GLint xoffset,
                            GLsizei width, GLenum format, GLenum type,
                            const GLvoid *pixels )
{
   GLvoid *image = _mesa_unpack_image(width, 1, 1, format, type,
                                      pixels, &gCCPriv->Unpack);
   Node *n = alloc_instruction( OPCODE_TEX_SUB_IMAGE1D, 7 );
   if (n) {
      n[1].e = target;
      n[2].i = level;
      n[3].i = xoffset;
      n[4].i = (GLint) width;
      n[5].e = format;
      n[6].e = type;
      n[7].data = image;
   } else 
   if (image) {
      free(image);
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexSubImage1D( target, level, xoffset, width,
                           format, type, image );
   }
}


void gl_save_TexSubImage2D( 
                            GLenum target, GLint level,
                            GLint xoffset, GLint yoffset,
                            GLsizei width, GLsizei height,
                            GLenum format, GLenum type,
                            const GLvoid *pixels )
{
   GLvoid *image = _mesa_unpack_image(width, height, 1, format, type,
                                      pixels, &gCCPriv->Unpack);
   Node *n = alloc_instruction( OPCODE_TEX_SUB_IMAGE2D, 9 );
   if (n) {
      n[1].e = target;
      n[2].i = level;
      n[3].i = xoffset;
      n[4].i = yoffset;
      n[5].i = (GLint) width;
      n[6].i = (GLint) height;
      n[7].e = format;
      n[8].e = type;
      n[9].data = image;
   } else 
   if (image) {
      free(image);
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_TexSubImage2D( target, level, xoffset, yoffset,
                           width, height, format, type, image );
   }
}


void gl_save_Translatef( GLfloat x, GLfloat y, GLfloat z )
{
   Node *n = alloc_instruction( OPCODE_TRANSLATE, 3 );
   if (n) {
      n[1].f = x;
      n[2].f = y;
      n[3].f = z;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Translatef( x, y, z );
   }
}


void gl_save_Vertex2f( GLfloat x, GLfloat y )
{
   Node *n = alloc_instruction( OPCODE_VERTEX2, 2 );
   if (n) {
      n[1].f = x;
      n[2].f = y;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Vertex2f( x, y );
   }
}


void gl_save_Vertex3f( GLfloat x, GLfloat y, GLfloat z )
{
   Node *n = alloc_instruction( OPCODE_VERTEX3, 3 );

   if (n) {
      n[1].f = x;
      n[2].f = y;
      n[3].f = z;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Vertex3f( x, y, z );
   }
}


void gl_save_Vertex4f( 
                       GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   Node *n = alloc_instruction( OPCODE_VERTEX4, 4 );
   if (n) {
      n[1].f = x;
      n[2].f = y;
      n[3].f = z;
      n[4].f = w;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Vertex4f( x, y, z, w );
   }
}


void gl_save_Vertex3fv( const GLfloat v[3] )
{
   Node *n = alloc_instruction( OPCODE_VERTEX3, 3 );
   if (n) {
      n[1].f = v[0];
      n[2].f = v[1];
      n[3].f = v[2];
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Vertex3fv( v );
   }
}


void gl_save_Viewport( 
                       GLint x, GLint y, GLsizei width, GLsizei height )
{
   Node *n = alloc_instruction( OPCODE_VIEWPORT, 4 );
   if (n) {
      n[1].i = x;
      n[2].i = y;
      n[3].i = (GLint) width;
      n[4].i = (GLint) height;
   }
   if (gCCPriv->ExecuteFlag) {
      _gamma_Viewport( x, y, width, height );
   }
}


/**********************************************************************/
/*                     Display list execution                         */
/**********************************************************************/


/*
 * Execute a display list.  Note that the ListBase offset must have already
 * been added before calling this function.  I.e. the list argument is
 * the absolute list number, not relative to ListBase.
 * Input:  list - display list number
 */
static void execute_list( GLuint list )
{
   Node *n;
   GLboolean done;
   OpCode opcode;

   if (!_gamma_IsList(list))
      return;

   gCCPriv->CallDepth++;

   n = (Node *) _mesa_HashLookup(gCCPriv->DisplayList, list);

   done = GL_FALSE;
   while (!done) {
      opcode = n[0].opcode;

      switch (opcode) {
	 /* Frequently called functions: */
         case OPCODE_VERTEX2:
            _gamma_Vertex2f( n[1].f, n[2].f );
            break;
         case OPCODE_VERTEX3:
            _gamma_Vertex3f( n[1].f, n[2].f, n[3].f );
            break;
         case OPCODE_VERTEX4:
            _gamma_Vertex4f( n[1].f, n[2].f, n[3].f, n[4].f );
            break;
         case OPCODE_NORMAL:
            _gamma_Normal3f( n[1].f, n[2].f, n[3].f );
#if 0 /* NOT_DONE */
            gCCPriv->VB->MonoNormal = GL_FALSE;
#endif
            break;
	 case OPCODE_COLOR_4UB:
            _gamma_Color4ub( n[1].ub, n[2].ub, n[3].ub, n[4].ub );
	    break;
	 case OPCODE_COLOR_3F:
            _gamma_Color3f( n[1].f, n[2].f, n[3].f );
            break;
	 case OPCODE_COLOR_4F:
            _gamma_Color4f( n[1].f, n[2].f, n[3].f, n[4].f );
            break;
         case OPCODE_INDEX:
            gCCPriv->Current.Index = n[1].ui;
#if 0 /* NOT_DONE */
            gCCPriv->VB->MonoColor = GL_FALSE;
#endif
            break;
         case OPCODE_BEGIN:
            _gamma_Begin( n[1].e );
            break;
         case OPCODE_END:
            _gamma_End( );
            break;
	 case OPCODE_TEXCOORD2:
	    _gamma_TexCoord2f( n[1].f, n[2].f );
	    break;
	 case OPCODE_TEXCOORD4:
	    _gamma_TexCoord4f( n[1].f, n[2].f, n[3].f, n[4].f );
	    break;
         case OPCODE_ACCUM:
	    _gamma_Accum( n[1].e, n[2].f );
	    break;
         case OPCODE_ALPHA_FUNC:
	    _gamma_AlphaFunc( n[1].e, n[2].f );
	    break;
         case OPCODE_BIND_TEXTURE:
            _gamma_BindTexture( n[1].e, n[2].ui );
            break;
	 case OPCODE_BITMAP:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_Bitmap( (GLsizei) n[1].i, (GLsizei) n[2].i,
                 n[3].f, n[4].f, n[5].f, n[6].f, (const GLubyte *) n[7].data );
               gCCPriv->Unpack = save;  /* restore */
            }
	    break;
	 case OPCODE_BLEND_FUNC:
	    _gamma_BlendFunc( n[1].e, n[2].e );
	    break;
         case OPCODE_CALL_LIST:
	    /* Generated by glCallList(), don't add ListBase */
            if (gCCPriv->CallDepth<MAX_LIST_NESTING) {
               execute_list( n[1].ui );
            }
            break;
         case OPCODE_CALL_LIST_OFFSET:
	    /* Generated by glCallLists() so we must add ListBase */
            if (gCCPriv->CallDepth<MAX_LIST_NESTING) {
               execute_list( gCCPriv->List.ListBase + n[1].ui );
            }
            break;
	 case OPCODE_CLEAR:
	    _gamma_Clear( n[1].bf );
	    break;
	 case OPCODE_CLEAR_COLOR:
	    _gamma_ClearColor( n[1].f, n[2].f, n[3].f, n[4].f );
	    break;
	 case OPCODE_CLEAR_ACCUM:
	    _gamma_ClearAccum( n[1].f, n[2].f, n[3].f, n[4].f );
	    break;
	 case OPCODE_CLEAR_DEPTH:
	    _gamma_ClearDepth( (GLclampd) n[1].f );
	    break;
	 case OPCODE_CLEAR_INDEX:
	    _gamma_ClearIndex( n[1].ui );
	    break;
	 case OPCODE_CLEAR_STENCIL:
	    _gamma_ClearStencil( n[1].i );
	    break;
         case OPCODE_CLIP_PLANE:
            {
               GLdouble equ[4];
               equ[0] = n[2].f;
               equ[1] = n[3].f;
               equ[2] = n[4].f;
               equ[3] = n[5].f;
               _gamma_ClipPlane( n[1].e, equ );
            }
            break;
	 case OPCODE_COLOR_MASK:
	    _gamma_ColorMask( n[1].b, n[2].b, n[3].b, n[4].b );
	    break;
	 case OPCODE_COLOR_MATERIAL:
	    _gamma_ColorMaterial( n[1].e, n[2].e );
	    break;
	 case OPCODE_COPY_PIXELS:
	    _gamma_CopyPixels( n[1].i, n[2].i,
			   (GLsizei) n[3].i, (GLsizei) n[4].i, n[5].e );
	    break;
         case OPCODE_COPY_TEX_IMAGE1D:
            _gamma_CopyTexImage1D( n[1].e, n[2].i, n[3].e, n[4].i,
                                         n[5].i, n[6].i, n[7].i );
            break;
         case OPCODE_COPY_TEX_IMAGE2D:
	    _gamma_CopyTexImage2D( n[1].e, n[2].i, n[3].e, n[4].i,
                               n[5].i, n[6].i, n[7].i, n[8].i );
            break;
         case OPCODE_COPY_TEX_SUB_IMAGE1D:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_TexSubImage1D( n[1].e, n[2].i, n[3].i,
                                           n[4].i, n[5].e,
                                           n[6].e, n[7].data );
               gCCPriv->Unpack = save;  /* restore */
            }
            break;
         case OPCODE_COPY_TEX_SUB_IMAGE2D:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_TexSubImage2D( n[1].e, n[2].i, n[3].i,
                                           n[4].i, n[5].e,
                                           n[6].i, n[7].e, n[8].e, n[9].data );
               gCCPriv->Unpack = save;  /* restore */
            }
            break;
	 case OPCODE_CULL_FACE:
	    _gamma_CullFace( n[1].e );
	    break;
	 case OPCODE_DEPTH_FUNC:
	    _gamma_DepthFunc( n[1].e );
	    break;
	 case OPCODE_DEPTH_MASK:
	    _gamma_DepthMask( n[1].b );
	    break;
	 case OPCODE_DEPTH_RANGE:
	    _gamma_DepthRange( (GLclampd) n[1].f, (GLclampd) n[2].f );
	    break;
	 case OPCODE_DISABLE:
	    _gamma_Disable( n[1].e );
	    break;
	 case OPCODE_DRAW_BUFFER:
	    _gamma_DrawBuffer( n[1].e );
	    break;
	 case OPCODE_DRAW_PIXELS:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_DrawPixels( n[1].i, n[2].i, n[3].e, n[4].e,
                                        n[5].data );
               gCCPriv->Unpack = save;  /* restore */
            }
	    break;
	 case OPCODE_EDGE_FLAG:
            gCCPriv->Current.EdgeFlag = n[1].b;
            break;
	 case OPCODE_ENABLE:
	    _gamma_Enable( n[1].e );
	    break;
	 case OPCODE_EVALCOORD1:
	    _gamma_EvalCoord1f( n[1].f );
	    break;
	 case OPCODE_EVALCOORD2:
	    _gamma_EvalCoord2f( n[1].f, n[2].f );
	    break;
	 case OPCODE_EVALMESH1:
	    _gamma_EvalMesh1( n[1].e, n[2].i, n[3].i );
	    break;
	 case OPCODE_EVALMESH2:
	    _gamma_EvalMesh2( n[1].e, n[2].i, n[3].i, n[4].i, n[5].i );
	    break;
	 case OPCODE_EVALPOINT1:
	    _gamma_EvalPoint1( n[1].i );
	    break;
	 case OPCODE_EVALPOINT2:
	    _gamma_EvalPoint2( n[1].i, n[2].i );
	    break;
	 case OPCODE_FOG:
	    {
	       GLfloat p[4];
	       p[0] = n[2].f;
	       p[1] = n[3].f;
	       p[2] = n[4].f;
	       p[3] = n[5].f;
	       _gamma_Fogfv( n[1].e, p );
	    }
	    break;
	 case OPCODE_FRONT_FACE:
	    _gamma_FrontFace( n[1].e );
	    break;
         case OPCODE_FRUSTUM:
            _gamma_Frustum( n[1].f, n[2].f, n[3].f, n[4].f, n[5].f, n[6].f );
            break;
	 case OPCODE_HINT:
	    _gamma_Hint( n[1].e, n[2].e );
	    break;
	 case OPCODE_INDEX_MASK:
	    _gamma_IndexMask( n[1].ui );
	    break;
	 case OPCODE_INIT_NAMES:
	    _gamma_InitNames( );
	    break;
         case OPCODE_LIGHT:
	    {
	       GLfloat p[4];
	       p[0] = n[3].f;
	       p[1] = n[4].f;
	       p[2] = n[5].f;
	       p[3] = n[6].f;
	       _gamma_Lightfv( n[1].e, n[2].e, p );
	    }
	    break;
         case OPCODE_LIGHT_MODEL:
	    {
	       GLfloat p[4];
	       p[0] = n[2].f;
	       p[1] = n[3].f;
	       p[2] = n[4].f;
	       p[3] = n[5].f;
	       _gamma_LightModelfv( n[1].e, p );
	    }
	    break;
	 case OPCODE_LINE_STIPPLE:
	    _gamma_LineStipple( n[1].i, n[2].us );
	    break;
	 case OPCODE_LINE_WIDTH:
	    _gamma_LineWidth( n[1].f );
	    break;
	 case OPCODE_LIST_BASE:
	    _gamma_ListBase( n[1].ui );
	    break;
	 case OPCODE_LOAD_IDENTITY:
            _gamma_LoadIdentity( );
            break;
	 case OPCODE_LOAD_MATRIX:
	    if (sizeof(Node)==sizeof(GLfloat)) {
	       _gamma_LoadMatrixf( &n[1].f );
	    }
	    else {
	       GLfloat m[16];
	       GLuint i;
	       for (i=0;i<16;i++) {
		  m[i] = n[1+i].f;
	       }
	       _gamma_LoadMatrixf( m );
	    }
	    break;
	 case OPCODE_LOAD_NAME:
	    _gamma_LoadName( n[1].ui );
	    break;
	 case OPCODE_LOGIC_OP:
	    _gamma_LogicOp( n[1].e );
	    break;
	 case OPCODE_MAP1:
	    _gamma_Map1f( n[1].e, n[2].f, n[3].f,
                      n[4].i, n[5].i, (GLfloat *) n[6].data );
	    break;
	 case OPCODE_MAP2:
	    _gamma_Map2f( n[1].e,
                      n[2].f, n[3].f,  /* u1, u2 */
		      n[6].i, n[8].i,  /* ustride, uorder */
		      n[4].f, n[5].f,  /* v1, v2 */
		      n[7].i, n[9].i,  /* vstride, vorder */
		      (GLfloat *) n[10].data);
	    break;
	 case OPCODE_MAPGRID1:
	    _gamma_MapGrid1f( n[1].i, n[2].f, n[3].f );
	    break;
	 case OPCODE_MAPGRID2:
	    _gamma_MapGrid2f( n[1].i, n[2].f, n[3].f, n[4].i, n[5].f, n[6].f);
	    break;
	 case OPCODE_MATERIAL:
	    {
	       GLfloat params[4];
	       params[0] = n[3].f;
	       params[1] = n[4].f;
	       params[2] = n[5].f;
	       params[3] = n[6].f;
	       _gamma_Materialfv( n[1].e, n[2].e, params );
	    }
	    break;
         case OPCODE_MATRIX_MODE:
            _gamma_MatrixMode( n[1].e );
            break;
	 case OPCODE_MULT_MATRIX:
	    if (sizeof(Node)==sizeof(GLfloat)) {
	       _gamma_MultMatrixf( &n[1].f );
	    }
	    else {
	       GLfloat m[16];
	       GLuint i;
	       for (i=0;i<16;i++) {
		  m[i] = n[1+i].f;
	       }
	       _gamma_MultMatrixf( m );
	    }
	    break;
         case OPCODE_ORTHO:
            _gamma_Ortho( n[1].f, n[2].f, n[3].f, n[4].f, n[5].f, n[6].f );
            break;
	 case OPCODE_PASSTHROUGH:
	    _gamma_PassThrough( n[1].f );
	    break;
	 case OPCODE_PIXEL_MAP:
	    _gamma_PixelMapfv( n[1].e, n[2].i, (GLfloat *) n[3].data );
	    break;
	 case OPCODE_PIXEL_TRANSFER:
	    _gamma_PixelTransferf( n[1].e, n[2].f );
	    break;
	 case OPCODE_PIXEL_ZOOM:
	    _gamma_PixelZoom( n[1].f, n[2].f );
	    break;
	 case OPCODE_POINT_SIZE:
	    _gamma_PointSize( n[1].f );
	    break;
	 case OPCODE_POLYGON_MODE:
	    _gamma_PolygonMode( n[1].e, n[2].e );
	    break;
	 case OPCODE_POLYGON_STIPPLE:
	    _gamma_PolygonStipple( (GLubyte *) n[1].data );
	    break;
	 case OPCODE_POLYGON_OFFSET:
	    _gamma_PolygonOffset( n[1].f, n[2].f );
	    break;
	 case OPCODE_POP_ATTRIB:
	    _gamma_PopAttrib( );
	    break;
	 case OPCODE_POP_MATRIX:
	    _gamma_PopMatrix( );
	    break;
	 case OPCODE_POP_NAME:
	    _gamma_PopName( );
	    break;
	 case OPCODE_PRIORITIZE_TEXTURE:
            _gamma_PrioritizeTextures( 1, &n[1].ui, &n[2].f );
	    break;
	 case OPCODE_PUSH_ATTRIB:
	    _gamma_PushAttrib( n[1].bf );
	    break;
	 case OPCODE_PUSH_MATRIX:
	    _gamma_PushMatrix( );
	    break;
	 case OPCODE_PUSH_NAME:
	    _gamma_PushName( n[1].ui );
	    break;
	 case OPCODE_RASTER_POS:
            _gamma_RasterPos4f( n[1].f, n[2].f, n[3].f, n[4].f );
	    break;
	 case OPCODE_READ_BUFFER:
	    _gamma_ReadBuffer( n[1].e );
	    break;
         case OPCODE_RECTF:
            _gamma_Rectf( n[1].f, n[2].f, n[3].f, n[4].f );
            break;
         case OPCODE_SCALE:
            _gamma_Scalef( n[1].f, n[2].f, n[3].f );
            break;
	 case OPCODE_SCISSOR:
	    _gamma_Scissor( n[1].i, n[2].i, n[3].i, n[4].i );
	    break;
	 case OPCODE_SHADE_MODEL:
	    _gamma_ShadeModel( n[1].e );
	    break;
	 case OPCODE_STENCIL_FUNC:
	    _gamma_StencilFunc( n[1].e, n[2].i, n[3].ui );
	    break;
	 case OPCODE_STENCIL_MASK:
	    _gamma_StencilMask( n[1].ui );
	    break;
	 case OPCODE_STENCIL_OP:
	    _gamma_StencilOp( n[1].e, n[2].e, n[3].e );
	    break;
         case OPCODE_TEXENV:
            {
               GLfloat params[4];
               params[0] = n[3].f;
               params[1] = n[4].f;
               params[2] = n[5].f;
               params[3] = n[6].f;
               _gamma_TexEnvfv( n[1].e, n[2].e, params );
            }
            break;
         case OPCODE_TEXGEN:
            {
               GLfloat params[4];
               params[0] = n[3].f;
               params[1] = n[4].f;
               params[2] = n[5].f;
               params[3] = n[6].f;
               _gamma_TexGenfv( n[1].e, n[2].e, params );
            }
            break;
         case OPCODE_TEXPARAMETER:
            {
               GLfloat params[4];
               params[0] = n[3].f;
               params[1] = n[4].f;
               params[2] = n[5].f;
               params[3] = n[6].f;
               _gamma_TexParameterfv( n[1].e, n[2].e, params );
            }
            break;
	 case OPCODE_TEX_IMAGE1D:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_TexImage1D(      n[1].e, /* target */
                                       n[2].i, /* level */
                                       n[3].i, /* components */
                                       n[4].i, /* width */
                                       n[5].e, /* border */
                                       n[6].e, /* format */
                                       n[7].e, /* type */
                                       n[8].data );
               gCCPriv->Unpack = save;  /* restore */
            }
	    break;
	 case OPCODE_TEX_IMAGE2D:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_TexImage2D(      n[1].e, /* target */
                                       n[2].i, /* level */
                                       n[3].i, /* components */
                                       n[4].i, /* width */
                                       n[5].i, /* height */
                                       n[6].e, /* border */
                                       n[7].e, /* format */
                                       n[8].e, /* type */
                                       n[9].data );
               gCCPriv->Unpack = save;  /* restore */
            }
            break;
         case OPCODE_TEX_SUB_IMAGE1D:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_TexSubImage1D( n[1].e, n[2].i, n[3].i,
                                           n[4].i, n[5].e,
                                           n[6].e, n[7].data );
               gCCPriv->Unpack = save;  /* restore */
            }
            break;
         case OPCODE_TEX_SUB_IMAGE2D:
            {
               struct gl_pixelstore_attrib save = gCCPriv->Unpack;
               gCCPriv->Unpack = _mesa_native_packing;
               _gamma_TexSubImage2D( n[1].e, n[2].i, n[3].i,
                                           n[4].i, n[5].e,
                                           n[6].i, n[7].e, n[8].e, n[9].data );
               gCCPriv->Unpack = save;  /* restore */
            }
            break;
         case OPCODE_TRANSLATE:
            _gamma_Translatef( n[1].f, n[2].f, n[3].f );
            break;
	 case OPCODE_VIEWPORT:
            _gamma_Viewport( n[1].i, n[2].i, (GLsizei) n[3].i, (GLsizei) n[4].i );
	    break;
	 case OPCODE_CONTINUE:
	    n = (Node *) n[1].next;
	    break;
	 case OPCODE_END_OF_LIST:
	    done = GL_TRUE;
	    break;
	 default:
            {
               char msg[1000];
               sprintf(msg, "Error in execute_list: opcode=%d", (int) opcode);
#if 0
               gl_problem( msg );
#endif
            }
            done = GL_TRUE;
      }

      /* increment n to point to next compiled command */
      if (opcode!=OPCODE_CONTINUE) {
	 n += InstSize[opcode];
      }

   }
   gCCPriv->CallDepth--;
}



/**********************************************************************/
/*                           GL functions                             */
/**********************************************************************/



/*
 * Test if a display list number is valid.
 */
GLboolean _gamma_IsList( GLuint list )
{
   if (list > 0 && _mesa_HashLookup(gCCPriv->DisplayList, list)) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



/*
 * Delete a sequence of consecutive display lists.
 */
void _gamma_DeleteLists(  GLuint list, GLsizei range )
{
   GLuint i;

#if 0
   if (INSIDE_BEGIN_END(ctx)) {
      gamma_error( GL_INVALID_OPERATION, "glDeleteLists" );
      return;
   }
#endif
   if (range<0) {
      gamma_error( GL_INVALID_VALUE, "glDeleteLists" );
      return;
   }
   for (i=list;i<list+range;i++) {
      gamma_destroy_list( i );
   }
}



/*
 * Return a display list number, n, such that lists n through n+range-1
 * are free.
 */
GLuint _gamma_GenLists( GLsizei range )
{
   GLuint base;

#if 0
   if (INSIDE_BEGIN_END(ctx)) {
      gamma_error( GL_INVALID_OPERATION, "glGenLists" );
      return 0;
   }
#endif
   if (range<0) {
      gamma_error( GL_INVALID_VALUE, "glGenLists" );
      return 0;
   }
   if (range==0) {
      return 0;
   }

   base = _mesa_HashFindFreeKeyBlock(gCCPriv->DisplayList, range);
   if (base) {
      /* reserve the list IDs by with empty/dummy lists */
      GLint i;
      for (i=0; i<range; i++) {
         _mesa_HashInsert(gCCPriv->DisplayList, base+i, make_empty_list());
      }
   }
   return base;
}



/*
 * Begin a new display list.
 */
void _gamma_NewList( GLuint list, GLenum mode )
{
#if 0
   if (INSIDE_BEGIN_END(ctx)) {
      gamma_error( GL_INVALID_OPERATION, "glNewList" );
      return;
   }
#endif
   if (list==0) {
      gamma_error( GL_INVALID_VALUE, "glNewList" );
      return;
   }
   if (mode!=GL_COMPILE && mode!=GL_COMPILE_AND_EXECUTE) {
      gamma_error( GL_INVALID_ENUM, "glNewList" );
      return;
   }
   if (gCCPriv->CurrentListPtr) {
      /* already compiling a display list */
      gamma_error( GL_INVALID_OPERATION, "glNewList" );
      return;
   }

   /* Allocate new display list */
   gCCPriv->CurrentListNum = list;
   gCCPriv->CurrentListPtr = gCCPriv->CurrentBlock = (Node *) malloc( sizeof(Node) * BLOCK_SIZE );
   gCCPriv->CurrentPos = 0;

   gCCPriv->CompileFlag = GL_TRUE;
   if (mode==GL_COMPILE) {
      gCCPriv->ExecuteFlag = GL_FALSE;
   }
   else {
      /* Compile and execute */
      gCCPriv->ExecuteFlag = GL_TRUE;
   }

   _glapi_set_dispatch(gCCPriv->Save);
   gCCPriv->API = gCCPriv->Save;  /* Switch the API function pointers */
}



/*
 * End definition of current display list.
 */
void _gamma_EndList( void )
{
   /* Check that a list is under construction */
   if (!gCCPriv->CurrentListPtr) {
      gamma_error( GL_INVALID_OPERATION, "glEndList" );
      return;
   }

   (void) alloc_instruction( OPCODE_END_OF_LIST, 0 );

   /* Destroy old list, if any */
   gamma_destroy_list(gCCPriv->CurrentListNum);
   /* Install the list */
   _mesa_HashInsert(gCCPriv->DisplayList, gCCPriv->CurrentListNum, gCCPriv->CurrentListPtr);

   gCCPriv->CurrentListNum = 0;
   gCCPriv->CurrentListPtr = NULL;
   gCCPriv->ExecuteFlag = GL_TRUE;
   gCCPriv->CompileFlag = GL_FALSE;

   _glapi_set_dispatch(gCCPriv->Exec);
   gCCPriv->API = gCCPriv->Exec;   /* Switch the API function pointers */
}



void _gamma_CallList( GLuint list )
{
   /* VERY IMPORTANT:  Save the CompileFlag status, turn it off, */
   /* execute the display list, and restore the CompileFlag. */
   GLboolean save_compile_flag;
   save_compile_flag = gCCPriv->CompileFlag;
   gCCPriv->CompileFlag = GL_FALSE;
   execute_list( list );
   gCCPriv->CompileFlag = save_compile_flag;

   /* also restore API function pointers to point to "save" versions */
   if (save_compile_flag) {
   	_glapi_set_dispatch(gCCPriv->Save);
        gCCPriv->API = gCCPriv->Save;
   }
}



/*
 * Execute glCallLists:  call multiple display lists.
 */
void _gamma_CallLists( 
                   GLsizei n, GLenum type, const GLvoid *lists )
{
   GLuint list;
   GLint i;
   GLboolean save_compile_flag;

   /* Save the CompileFlag status, turn it off, execute display list,
    * and restore the CompileFlag.
    */
   save_compile_flag = gCCPriv->CompileFlag;
   gCCPriv->CompileFlag = GL_FALSE;

   for (i=0;i<n;i++) {
      list = translate_id( i, type, lists );
      execute_list( gCCPriv->List.ListBase + list );
   }

   gCCPriv->CompileFlag = save_compile_flag;

   /* also restore API function pointers to point to "save" versions */
   if (save_compile_flag) {
   	_glapi_set_dispatch(gCCPriv->Save);
        gCCPriv->API = gCCPriv->Save;
   }
}



/*
 * Set the offset added to list numbers in glCallLists.
 */
void _gamma_ListBase( GLuint base )
{
#if 0
   if (INSIDE_BEGIN_END(ctx)) {
      gamma_error( GL_INVALID_OPERATION, "glListBase" );
      return;
   }
#endif
   gCCPriv->List.ListBase = base;
}
