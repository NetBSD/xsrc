/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_tex.c,v 1.9 2001/04/10 16:07:52 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
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
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Gareth Hughes <gareth@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *   Michel Dänzer <michdaen@iiic.ethz.ch>
 *
 */

#include "r128_context.h"
#include "r128_state.h"
#include "r128_ioctl.h"
#include "r128_vb.h"
#include "r128_tex.h"

#include "mmath.h"
#include "simple_list.h"
#include "enums.h"
#include "mem.h"

#define TEX_0	1
#define TEX_1	2


static void r128SetTexWrap( r128TexObjPtr t, GLenum swrap, GLenum twrap )
{
   t->setup.tex_cntl &= ~(R128_TEX_CLAMP_S_MASK | R128_TEX_CLAMP_T_MASK);

   switch ( swrap ) {
   case GL_CLAMP:
      t->setup.tex_cntl |= R128_TEX_CLAMP_S_CLAMP;
      break;
   case GL_REPEAT:
      t->setup.tex_cntl |= R128_TEX_CLAMP_S_WRAP;
      break;
   }

   switch ( twrap ) {
   case GL_CLAMP:
      t->setup.tex_cntl |= R128_TEX_CLAMP_T_CLAMP;
      break;
   case GL_REPEAT:
      t->setup.tex_cntl |= R128_TEX_CLAMP_T_WRAP;
      break;
   }
}

static void r128SetTexFilter( r128TexObjPtr t, GLenum minf, GLenum magf )
{
   t->setup.tex_cntl &= ~(R128_MIN_BLEND_MASK | R128_MAG_BLEND_MASK);

   switch ( minf ) {
   case GL_NEAREST:
      t->setup.tex_cntl |= R128_MIN_BLEND_NEAREST;
      break;
   case GL_LINEAR:
      t->setup.tex_cntl |= R128_MIN_BLEND_LINEAR;
      break;
   case GL_NEAREST_MIPMAP_NEAREST:
      t->setup.tex_cntl |= R128_MIN_BLEND_MIPNEAREST;
      break;
   case GL_LINEAR_MIPMAP_NEAREST:
      t->setup.tex_cntl |= R128_MIN_BLEND_LINEARMIPNEAREST;
      break;
   case GL_NEAREST_MIPMAP_LINEAR:
      t->setup.tex_cntl |= R128_MIN_BLEND_MIPLINEAR;
      break;
   case GL_LINEAR_MIPMAP_LINEAR:
      t->setup.tex_cntl |= R128_MIN_BLEND_LINEARMIPLINEAR;
      break;
   }

   switch ( magf ) {
   case GL_NEAREST:
      t->setup.tex_cntl |= R128_MAG_BLEND_NEAREST;
      break;
   case GL_LINEAR:
      t->setup.tex_cntl |= R128_MAG_BLEND_LINEAR;
      break;
   }
}

static void r128SetTexBorderColor( r128TexObjPtr t, GLubyte c[4] )
{
   t->setup.tex_border_color = r128PackColor( 4, c[0], c[1], c[2], c[3] );
}


/* Allocate and initialize hardware state associated with texture `t'.
 */
static r128TexObjPtr r128CreateTexObj( r128ContextPtr rmesa,
				       struct gl_texture_object *tObj )
{
   r128TexObjPtr t;
   struct gl_texture_image *image;
   int log2Pitch, log2Height, log2Size, log2MinSize;
   int totalSize;
   int i;

   image = tObj->Image[0];
   if ( !image )
      return NULL;

   t = (r128TexObjPtr) CALLOC( sizeof(*t) );
   if ( !t )
      return NULL;

   if ( R128_DEBUG & DEBUG_VERBOSE_API )
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, tObj );

   switch ( image->Format ) {
   case GL_RGBA:
   case GL_ALPHA:
   case GL_LUMINANCE_ALPHA:
      if ( rmesa->r128Screen->cpp == 4 ) {
	 t->texelBytes = 4;
	 t->textureFormat = R128_DATATYPE_ARGB8888;
      } else {
	 t->texelBytes = 2;
	 t->textureFormat = R128_DATATYPE_ARGB4444;
      }
      break;

   case GL_RGB:
      if ( rmesa->r128Screen->cpp == 4 ) {
	 t->texelBytes = 4;
	 t->textureFormat = R128_DATATYPE_ARGB8888;
      } else {
	 t->texelBytes = 2;
	 t->textureFormat = R128_DATATYPE_RGB565;
      }
      break;

   case GL_LUMINANCE:
   case GL_INTENSITY:
      t->texelBytes = 1;
      t->textureFormat = R128_DATATYPE_RGB8;
      break;

   case GL_COLOR_INDEX:
      t->texelBytes = 1;
      t->textureFormat = R128_DATATYPE_CI8;
      break;

   default:
      fprintf( stderr, "%s: bad image->Format\n", __FUNCTION__ );
      FREE( t );
      return NULL;
   }

   /* Calculate dimensions in log domain */
   for ( i = 1, log2Height = 0 ; i < image->Height ; i *= 2 ) {
      log2Height++;
   }
   for ( i = 1, log2Pitch = 0 ;  i < image->Width ;  i *= 2 ) {
      log2Pitch++;
   }
   if ( image->Width > image->Height ) {
      log2Size = log2Pitch;
   } else {
      log2Size = log2Height;
   }

   t->dirty_images = 0;

   /* Calculate mipmap offsets and dimensions */
   totalSize = 0;
   for ( i = 0 ; i <= log2Size && tObj->Image[i] ; i++ ) {
      t->image[i].offset = totalSize;
      t->image[i].width  = tObj->Image[i]->Width;
      t->image[i].height = tObj->Image[i]->Height;

      t->dirty_images |= (1 << i);

      totalSize += (tObj->Image[i]->Height *
		    tObj->Image[i]->Width *
		    t->texelBytes);

      /* Offsets must be 32-byte aligned for host data blits and tiling */
      totalSize = (totalSize + 31) & ~31;
   }
   log2MinSize = log2Size - i + 1;

   t->totalSize = totalSize;
   t->internFormat = image->IntFormat;

   t->age = 0;
   t->bound = 0;
   t->heap = 0;
   t->tObj = tObj;

   t->memBlock = NULL;
   t->bufAddr = 0;

   /* Hardware state:
    */
   t->setup.tex_cntl = (R128_MIN_BLEND_NEAREST |
			R128_MAG_BLEND_NEAREST |
			R128_TEX_CLAMP_S_WRAP |
			R128_TEX_CLAMP_T_WRAP |
			t->textureFormat);

   t->setup.tex_combine_cntl = 0x00000000;

   t->setup.tex_size_pitch = ((log2Pitch   << R128_TEX_PITCH_SHIFT) |
			      (log2Size    << R128_TEX_SIZE_SHIFT) |
			      (log2Height  << R128_TEX_HEIGHT_SHIFT) |
			      (log2MinSize << R128_TEX_MIN_SIZE_SHIFT));

   for ( i = 0 ; i < R128_MAX_TEXTURE_LEVELS ; i++ ) {
      t->setup.tex_offset[i]  = 0x00000000;
   }
   t->setup.tex_border_color = 0x00000000;

   if ( ( log2MinSize == log2Size ) || ( log2MinSize != 0 ) ) {
      t->setup.tex_cntl |= R128_MIP_MAP_DISABLE;
   }

   r128SetTexWrap( t, tObj->WrapS, tObj->WrapT );
   r128SetTexFilter( t, tObj->MinFilter, tObj->MagFilter );
   r128SetTexBorderColor( t, tObj->BorderColor );

   tObj->DriverData = t;

   make_empty_list( t );

   return t;
}

/* Destroy hardware state associated with texture `t'.
 */
void r128DestroyTexObj( r128ContextPtr rmesa, r128TexObjPtr t )
{
#if ENABLE_PERF_BOXES
   /* Bump the performace counter */
   rmesa->c_textureSwaps++;
#endif
   if ( !t ) return;

   if ( t->memBlock ) {
      mmFreeMem( t->memBlock );
      t->memBlock = NULL;
   }

   if ( t->tObj )
      t->tObj->DriverData = NULL;

   if ( t->bound & TEX_0 ) rmesa->CurrentTexObj[0] = NULL;
   if ( t->bound & TEX_1 ) rmesa->CurrentTexObj[1] = NULL;

   remove_from_list( t );
   FREE( t );
}

/* Keep track of swapped out texture objects.
 */
static void r128SwapOutTexObj( r128ContextPtr rmesa, r128TexObjPtr t )
{
#if ENABLE_PERF_BOXES
   /* Bump the performace counter */
   rmesa->c_textureSwaps++;
#endif
   if ( t->memBlock ) {
      mmFreeMem( t->memBlock );
      t->memBlock = NULL;
   }

   t->dirty_images = ~0;
   move_to_tail( &rmesa->SwappedOut, t );
}

/* Print out debugging information about texture LRU.
 */
void r128PrintLocalLRU( r128ContextPtr rmesa, int heap )
{
   r128TexObjPtr t;
   int sz = 1 << (rmesa->r128Screen->logTexGranularity[heap]);

   fprintf( stderr, "\nLocal LRU, heap %d:\n", heap );

   foreach ( t, &rmesa->TexObjList[heap] ) {
      if ( !t->tObj ) {
	 fprintf( stderr, "Placeholder %d at 0x%x sz 0x%x\n",
		  t->memBlock->ofs / sz,
		  t->memBlock->ofs,
		  t->memBlock->size );
      } else {
	 fprintf( stderr, "Texture (bound %d) at 0x%x sz 0x%x\n",
		  t->bound,
		  t->memBlock->ofs,
		  t->memBlock->size );
      }
   }

   fprintf( stderr, "\n" );
}

void r128PrintGlobalLRU( r128ContextPtr rmesa, int heap )
{
   r128_tex_region_t *list = rmesa->sarea->texList[heap];
   int i, j;

   fprintf( stderr, "\nGlobal LRU, heap %d list %p:\n", heap, list );

   for ( i = 0, j = R128_NR_TEX_REGIONS ; i < R128_NR_TEX_REGIONS ; i++ ) {
      fprintf( stderr, "list[%d] age %d next %d prev %d\n",
	       j, list[j].age, list[j].next, list[j].prev );
      j = list[j].next;
      if ( j == R128_NR_TEX_REGIONS ) break;
   }

   if ( j != R128_NR_TEX_REGIONS ) {
      fprintf( stderr, "Loop detected in global LRU\n" );
      for ( i = 0 ; i < R128_NR_TEX_REGIONS ; i++ ) {
	 fprintf( stderr, "list[%d] age %d next %d prev %d\n",
		  i, list[i].age, list[i].next, list[i].prev );
      }
   }

   fprintf( stderr, "\n" );
}

/* Reset the global texture LRU.
 */
static void r128ResetGlobalLRU( r128ContextPtr rmesa, int heap )
{
   r128_tex_region_t *list = rmesa->sarea->texList[heap];
   int sz = 1 << rmesa->r128Screen->logTexGranularity[heap];
   int i;

   /* (Re)initialize the global circular LRU list.  The last element in
    * the array (R128_NR_TEX_REGIONS) is the sentinal.  Keeping it at
    * the end of the array allows it to be addressed rationally when
    * looking up objects at a particular location in texture memory.
    */
   for ( i = 0 ; (i+1) * sz <= rmesa->r128Screen->texSize[heap] ; i++ ) {
      list[i].prev = i-1;
      list[i].next = i+1;
      list[i].age = 0;
   }

   i--;
   list[0].prev = R128_NR_TEX_REGIONS;
   list[i].prev = i-1;
   list[i].next = R128_NR_TEX_REGIONS;
   list[R128_NR_TEX_REGIONS].prev = i;
   list[R128_NR_TEX_REGIONS].next = 0;
   rmesa->sarea->texAge[heap] = 0;
}

/* Update the local and glock texture LRUs.
 */
static void r128UpdateTexLRU( r128ContextPtr rmesa, r128TexObjPtr t )
{
   int heap = t->heap;
   r128_tex_region_t *list = rmesa->sarea->texList[heap];
   int log2sz = rmesa->r128Screen->logTexGranularity[heap];
   int start = t->memBlock->ofs >> log2sz;
   int end = (t->memBlock->ofs + t->memBlock->size - 1) >> log2sz;
   int i;

   rmesa->lastTexAge[heap] = ++rmesa->sarea->texAge[heap];

   if ( !t->memBlock ) {
      fprintf( stderr, "no memblock\n\n" );
      return;
   }

   /* Update our local LRU */
   move_to_head( &rmesa->TexObjList[heap], t );

   /* Update the global LRU */
   for ( i = start ; i <= end ; i++ ) {
      list[i].in_use = 1;
      list[i].age = rmesa->lastTexAge[heap];

      /* remove_from_list(i) */
      list[(CARD32)list[i].next].prev = list[i].prev;
      list[(CARD32)list[i].prev].next = list[i].next;

      /* insert_at_head(list, i) */
      list[i].prev = R128_NR_TEX_REGIONS;
      list[i].next = list[R128_NR_TEX_REGIONS].next;
      list[(CARD32)list[R128_NR_TEX_REGIONS].next].prev = i;
      list[R128_NR_TEX_REGIONS].next = i;
   }

   if ( 0 ) {
      r128PrintGlobalLRU( rmesa, t->heap );
      r128PrintLocalLRU( rmesa, t->heap );
   }
}

/* Update our notion of what textures have been changed since we last
 * held the lock.  This pertains to both our local textures and the
 * textures belonging to other clients.  Keep track of other client's
 * textures by pushing a placeholder texture onto the LRU list -- these
 * are denoted by (tObj == NULL).
 */
static void r128TexturesGone( r128ContextPtr rmesa, int heap,
			      int offset, int size, int in_use )
{
   r128TexObjPtr t, tmp;

   foreach_s ( t, tmp, &rmesa->TexObjList[heap] ) {
      if ( t->memBlock->ofs >= offset + size ||
	   t->memBlock->ofs + t->memBlock->size <= offset )
	 continue;

      /* It overlaps - kick it out.  Need to hold onto the currently
       * bound objects, however.
       */
      if ( t->bound ) {
	 r128SwapOutTexObj( rmesa, t );
      } else {
	 r128DestroyTexObj( rmesa, t );
      }
   }

   if ( in_use ) {
      t = (r128TexObjPtr) CALLOC( sizeof(*t) );
      if ( !t ) return;

      t->memBlock = mmAllocMem( rmesa->texHeap[heap], size, 0, offset );
      if ( !t->memBlock ) {
	 fprintf( stderr, "Couldn't alloc placeholder sz %x ofs %x\n",
		  (int)size, (int)offset );
	 mmDumpMemInfo( rmesa->texHeap[heap] );
	 return;
      }
      insert_at_head( &rmesa->TexObjList[heap], t );
   }
}

/* Update our client's shared texture state.  If another client has
 * modified a region in which we have textures, then we need to figure
 * out which of our textures has been removed, and update our global
 * LRU.
 */
void r128AgeTextures( r128ContextPtr rmesa, int heap )
{
   R128SAREAPrivPtr sarea = rmesa->sarea;

   if ( sarea->texAge[heap] != rmesa->lastTexAge[heap] ) {
      int sz = 1 << rmesa->r128Screen->logTexGranularity[heap];
      int nr = 0;
      int idx;

      /* Have to go right round from the back to ensure stuff ends up
       * LRU in our local list...  Fix with a cursor pointer.
       */
      for ( idx = sarea->texList[heap][R128_NR_TEX_REGIONS].prev ;
	    idx != R128_NR_TEX_REGIONS && nr < R128_NR_TEX_REGIONS ;
	    idx = sarea->texList[heap][idx].prev, nr++ )
      {
	 /* If switching texturing schemes, then the SAREA might not
	  * have been properly cleared, so we need to reset the
	  * global texture LRU.
	  */
	 if ( idx * sz > rmesa->r128Screen->texSize[heap] ) {
	    nr = R128_NR_TEX_REGIONS;
	    break;
	 }

	 if ( sarea->texList[heap][idx].age > rmesa->lastTexAge[heap] ) {
	    r128TexturesGone( rmesa, heap, idx * sz, sz,
			      sarea->texList[heap][idx].in_use );
	 }
      }

      /* If switching texturing schemes, then the SAREA might not
       * have been properly cleared, so we need to reset the
       * global texture LRU.
       */
      if ( nr == R128_NR_TEX_REGIONS ) {
	 r128TexturesGone( rmesa, heap, 0,
			   rmesa->r128Screen->texSize[heap], 0 );
	 r128ResetGlobalLRU( rmesa, heap );
      }

      if ( 0 ) {
	 r128PrintGlobalLRU( rmesa, heap );
	 r128PrintLocalLRU( rmesa, heap );
      }

      rmesa->dirty |= (R128_UPLOAD_CONTEXT |
		       R128_UPLOAD_TEX0IMAGES |
		       R128_UPLOAD_TEX1IMAGES);
      rmesa->lastTexAge[heap] = sarea->texAge[heap];
   }
}


/* ================================================================
 * Texture image conversions
 */

/* Convert a block of Mesa-formatted texture to an 8bpp hardware format.
 */
static void r128ConvertTexture8bpp( CARD32 *dst,
				    struct gl_texture_image *image,
				    int x, int y, int width, int height,
				    int pitch )
{
   CARD8 *src;
   int i, j;

   switch ( image->Format ) {
   case GL_LUMINANCE:
   case GL_INTENSITY:
   case GL_COLOR_INDEX:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x);
	 for ( j = width >> 2 ; j ; j-- ) {
	    *dst++ = src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
	    src += 4;
	 }
      }
      break;

   default:
      fprintf( stderr, "%s: unsupported format 0x%x\n",
	       __FUNCTION__, image->Format );
      break;
   }
}

/* Convert a block of Mesa-formatted texture to a 16bpp hardware format.
 */
static void r128ConvertTexture16bpp( CARD32 *dst,
				     struct gl_texture_image *image,
				     int x, int y, int width, int height,
				     int pitch )
{
   CARD8 *src;
   int i, j;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s: %d,%d at %d,%d\n",
	       __FUNCTION__, width, height, x, y );
   }


   switch ( image->Format ) {
   case GL_RGBA:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x) * 4;
	 for ( j = width >> 1 ; j ; j-- ) {
	    *dst++ = R128PACKCOLORS4444( src[0], src[1], src[2], src[3],
					 src[4], src[5], src[6], src[7] );
	    src += 8;
	 }
      }
      break;

   case GL_RGB:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x) * 3;
	 for ( j = width >> 1 ; j ; j-- ) {
	    *dst++ = R128PACKCOLORS565( src[0], src[1], src[2],
					src[3], src[4], src[5] );
	    src += 6;
	 }
      }
      break;

   case GL_ALPHA:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x);
	 for ( j = width >> 1 ; j ; j-- ) {
	    *dst++ = R128PACKCOLORS4444( 0xff, 0xff, 0xff, src[0],
					 0xff, 0xff, 0xff, src[1] );
	    src += 2;
	 }
      }
      break;

   case GL_LUMINANCE_ALPHA:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x) * 2;
	 for ( j = width >> 1 ; j ; j-- ) {
	    *dst++ = R128PACKCOLORS4444( src[0], src[0], src[0], src[1],
					 src[2], src[2], src[2], src[3] );
	    src += 4;
	 }
      }
      break;

   default:
      fprintf( stderr, "%s: unsupported format 0x%x\n",
	       __FUNCTION__, image->Format );
      break;
   }
}

/* Convert a block of Mesa-formatted texture to a 32bpp hardware format.
 */
static void r128ConvertTexture32bpp( CARD32 *dst,
				     struct gl_texture_image *image,
				     int x, int y, int width, int height,
				     int pitch )
{
   CARD8 *src;
   int i, j;

   switch ( image->Format ) {
   case GL_RGBA:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x) * 4;
	 for ( j = width ; j ; j-- ) {
	    *dst++ = R128PACKCOLOR8888( src[0], src[1], src[2], src[3] );
	    src += 4;
	 }
      }
      break;

   case GL_RGB:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x) * 3;
	 for ( j = width ; j ; j-- ) {
	    *dst++ = R128PACKCOLOR8888( src[0], src[1], src[2], 0xff );
	    src += 3;
	 }
      }
      break;

   case GL_ALPHA:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x);
	 for ( j = width ; j ; j-- ) {
	    *dst++ = R128PACKCOLOR8888( 0xff, 0xff, 0xff, src[0] );
	    src += 1;
	 }
      }
      break;

   case GL_LUMINANCE_ALPHA:
      for ( i = 0 ; i < height ; i++ ) {
	 src = (CARD8 *)image->Data + ((y + i) * pitch + x) * 2;
	 for ( j = width ; j ; j-- ) {
	    *dst++ = R128PACKCOLOR8888( src[0], src[0], src[0], src[1] );
	    src += 2;
	 }
      }
      break;

   default:
      fprintf( stderr, "%s: unsupported format 0x%x\n",
	       __FUNCTION__, image->Format );
      break;
   }
}

/* Upload the texture image associated with texture `t' at level `level'
 * at the address relative to `start'.
 */
static void r128UploadSubImage( r128ContextPtr rmesa,
				r128TexObjPtr t, int level,
				int x, int y, int width, int height )
{
   struct gl_texture_image *image;
   int texelsPerDword = 0;
   int imageWidth, imageHeight;
   int remaining, rows;
   int format, dwords;
   CARD32 pitch, offset;
   drmBufPtr buffer;
   CARD32 *dst;
   int i;

   /* Ensure we have a valid texture to upload */
   if ( ( level < 0 ) || ( level > R128_MAX_TEXTURE_LEVELS ) )
      return;

   image = t->tObj->Image[level];
   if ( !image )
      return;

   switch ( t->texelBytes ) {
   case 1: texelsPerDword = 4; break;
   case 2: texelsPerDword = 2; break;
   case 4: texelsPerDword = 1; break;
   }

#if 1
   /* FIXME: The subimage index calcs are wrong... */
   x = 0;
   y = 0;
   width = image->Width;
   height = image->Height;
#endif

   imageWidth  = image->Width;
   imageHeight = image->Height;

   format = t->textureFormat >> 16;

   /* The texel upload routines have a minimum width, so force the size
    * if needed.
    */
   if ( imageWidth < texelsPerDword ) {
      int factor;

      factor = texelsPerDword / imageWidth;
      imageWidth = texelsPerDword;
      imageHeight /= factor;
      if ( imageHeight == 0 ) {
	 /* In this case, the texel converter will actually walk a
	  * texel or two off the end of the image, but normal malloc
	  * alignment should prevent it from ever causing a fault.
	  */
	 imageHeight = 1;
      }
   }

   /* We can't upload to a pitch less than 8 texels so we will need to
    * linearly upload all modified rows for textures smaller than this.
    * This makes the x/y/width/height different for the blitter and the
    * texture walker.
    */
   if ( imageWidth >= 8 ) {
      /* The texture walker and the blitter look identical */
      pitch = imageWidth >> 3;
   } else {
      int factor;
      int y2;
      int start, end;

      start = (y * imageWidth) & ~7;
      end = (y + height) * imageWidth;

      if ( end - start < 8 ) {
	 /* Handle the case where the total number of texels
	  * uploaded is < 8.
	  */
	 x = 0;
	 y = start / 8;
	 width = end - start;
	 height = 1;
      } else {
	 /* Upload some number of full 8 texel blit rows */
	 factor = 8 / imageWidth;

	 y2 = y + height - 1;
	 y /= factor;
	 y2 /= factor;

	 x = 0;
	 width = 8;
	 height = y2 - y + 1;
      }

      /* Fixed pitch of 8 */
      pitch = 1;
   }

   dwords = width * height / texelsPerDword;
   offset = t->bufAddr + t->image[level].offset;

#if ENABLE_PERF_BOXES
   /* Bump the performace counter */
   rmesa->c_textureBytes += (dwords << 2);
#endif

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "r128UploadSubImage: %d,%d of %d,%d at %d,%d\n",
	       width, height, image->Width, image->Height, x, y );
      fprintf( stderr, "          blit ofs: 0x%07x pitch: 0x%x dwords: %d "
	       "level: %d format: %x\n",
	       (GLuint)offset, (GLuint)pitch, dwords, level, format );
   }

   /* Subdivide the texture if required */
   if ( dwords <= R128_BUFFER_MAX_DWORDS / 2 ) {
      rows = height;
   } else {
      rows = (R128_BUFFER_MAX_DWORDS * texelsPerDword) / (2 * width);
   }

   for ( i = 0, remaining = height ;
	 remaining > 0 ;
	 remaining -= rows, y += rows, i++ )
   {
      height = (remaining >= rows) ? rows : remaining;
      dwords = width * height / texelsPerDword;

      if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
	 fprintf( stderr, "          blitting: %d,%d at %d,%d - %d dwords\n",
		  width, height, x, y, dwords );
      }

      /* Grab the indirect buffer for the texture blit */
      buffer = r128GetBufferLocked( rmesa );

      dst = (CARD32 *)((char *)buffer->address + R128_HOSTDATA_BLIT_OFFSET);

      /* Actually do the texture conversion */
      switch ( t->texelBytes ) {
      case 1:
	 r128ConvertTexture8bpp( dst, image,
				 x, y, width, height, width );
	 break;
      case 2:
	 r128ConvertTexture16bpp( dst, image,
				  x, y, width, height, width );
	 break;
      case 4:
	 r128ConvertTexture32bpp( dst, image,
				  x, y, width, height, width );
	 break;
      }

      r128FireBlitLocked( rmesa, buffer,
			  offset, pitch, format,
			  x, y, width, height );
   }

   rmesa->new_state |= R128_NEW_CONTEXT;
   rmesa->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_MASKS;
}

/* Upload the texture images associated with texture `t'.  This might
 * require removing our own and/or other client's texture objects to
 * make room for these images.
 */
/* NOTE: This function is only called while holding the hardware lock */
int r128UploadTexImages( r128ContextPtr rmesa, r128TexObjPtr t )
{
   int i;
   int minLevel;
   int maxLevel;
   int heap;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %p )\n",
	       __FUNCTION__, rmesa->glCtx, t );
   }

   if ( !t ) return 0;

   /* Choose the heap appropriately */
   heap = t->heap = R128_CARD_HEAP;
   if ( !rmesa->r128Screen->IsPCI &&
	t->totalSize > rmesa->r128Screen->texSize[heap] ) {
      heap = t->heap = R128_AGP_HEAP;
   }

   /* Do we need to eject LRU texture objects? */
   if ( !t->memBlock ) {
      /* Allocate a memory block on a 4k boundary (1<<12 == 4096) */
      t->memBlock = mmAllocMem( rmesa->texHeap[heap], t->totalSize, 12, 0 );

      /* Try AGP before kicking anything out of local mem */
      if ( !rmesa->r128Screen->IsPCI &&
	   !t->memBlock && heap == R128_CARD_HEAP ) {
	 t->memBlock = mmAllocMem( rmesa->texHeap[R128_AGP_HEAP],
				   t->totalSize, 12, 0 );

	 if ( t->memBlock )
	    heap = t->heap = R128_AGP_HEAP;
      }

      /* Kick out textures until the requested texture fits */
      while ( !t->memBlock ) {
	 if ( rmesa->TexObjList[heap].prev->bound ) {
	    fprintf( stderr,
		     "r128UploadTexImages: ran into bound texture\n" );
	    return -1;
	 }
	 if ( rmesa->TexObjList[heap].prev == &rmesa->TexObjList[heap] ) {
	    if ( rmesa->r128Screen->IsPCI ) {
	       fprintf( stderr, "r128UploadTexImages: upload texture "
			"failure on local texture heaps, sz=%d\n",
			t->totalSize );
	       return -1;
	    } else if ( heap == R128_CARD_HEAP ) {
	       heap = t->heap = R128_AGP_HEAP;
	       continue;
	    } else {
	       fprintf( stderr, "r128UploadTexImages: upload texture "
			"failure on both local and AGP texture heaps, "
			"sz=%d\n",
			t->totalSize );
	       return -1;
	    }
	 }

	 r128DestroyTexObj( rmesa, rmesa->TexObjList[heap].prev );

	 t->memBlock = mmAllocMem( rmesa->texHeap[heap],
				   t->totalSize, 12, 0 );
      }

      /* Set the base offset of the texture image */
      t->bufAddr = rmesa->r128Screen->texOffset[heap] + t->memBlock->ofs;

      maxLevel = ((t->setup.tex_size_pitch & R128_TEX_SIZE_MASK) >>
		  R128_TEX_SIZE_SHIFT);
      minLevel = ((t->setup.tex_size_pitch & R128_TEX_MIN_SIZE_MASK) >>
		  R128_TEX_MIN_SIZE_SHIFT);

      /* Set texture offsets */
      if ( t->setup.tex_cntl & R128_MIP_MAP_DISABLE ) {
	 for ( i = 0 ; i < R128_MAX_TEXTURE_LEVELS ; i++ ) {
	    t->setup.tex_offset[i] = t->bufAddr;
	 }
      } else {
	 for ( i = maxLevel ; i >= minLevel ; i-- ) {
	    t->setup.tex_offset[i] =
	       (t->image[maxLevel-i].offset + t->bufAddr);
	 }
      }

      /* Force loading the new state into the hardware */
      switch ( t->bound ) {
      case 1:
	 rmesa->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_TEX0;
	 break;

      case 2:
	 rmesa->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_TEX1;
	 break;

      default:
	 return -1;
      }
   }

   /* Let the world know we've used this memory recently */
   r128UpdateTexLRU( rmesa, t );

   /* Upload any images that are new */
   if ( t->dirty_images ) {
      int num_levels = (((t->setup.tex_size_pitch & R128_TEX_SIZE_MASK) >>
			 R128_TEX_SIZE_SHIFT) -
			((t->setup.tex_size_pitch & R128_TEX_MIN_SIZE_MASK) >>
			 R128_TEX_MIN_SIZE_SHIFT));

      for ( i = 0 ; i <= num_levels ; i++ ) {
	 if ( t->dirty_images & (1 << i) ) {
	    r128UploadSubImage( rmesa, t, i, 0, 0,
				t->image[i].width, t->image[i].height );
	 }
      }

      rmesa->setup.tex_cntl_c |= R128_TEX_CACHE_FLUSH;

      rmesa->dirty |= R128_UPLOAD_CONTEXT;
   }

   t->dirty_images = 0;
   return 0;
}


/* ================================================================
 * Texture combine functions
 */

#define COLOR_COMB_DISABLE		(R128_COMB_DIS |		\
					 R128_COLOR_FACTOR_TEX)
#define COLOR_COMB_COPY_INPUT		(R128_COMB_COPY_INP |		\
					 R128_COLOR_FACTOR_TEX)
#define COLOR_COMB_MODULATE		(R128_COMB_MODULATE |		\
					 R128_COLOR_FACTOR_TEX)
#define COLOR_COMB_MODULATE_NTEX	(R128_COMB_MODULATE |		\
					 R128_COLOR_FACTOR_NTEX)
#define COLOR_COMB_ADD			(R128_COMB_ADD |		\
					 R128_COLOR_FACTOR_TEX)
#define COLOR_COMB_BLEND_TEX		(R128_COMB_BLEND_TEXTURE |	\
					 R128_COLOR_FACTOR_TEX)
/* Rage 128 Pro/M3 only! */
#define COLOR_COMB_BLEND_COLOR		(R128_COMB_MODULATE2X |		\
					 R128_COMB_FCN_MSB |		\
					 R128_COLOR_FACTOR_CONST_COLOR)

#define ALPHA_COMB_DISABLE		(R128_COMB_ALPHA_DIS |		\
					 R128_ALPHA_FACTOR_TEX_ALPHA)
#define ALPHA_COMB_COPY_INPUT		(R128_COMB_ALPHA_COPY_INP |	\
					 R128_ALPHA_FACTOR_TEX_ALPHA)
#define ALPHA_COMB_MODULATE		(R128_COMB_ALPHA_MODULATE |	\
					 R128_ALPHA_FACTOR_TEX_ALPHA)
#define ALPHA_COMB_MODULATE_NTEX	(R128_COMB_ALPHA_MODULATE |	\
					 R128_ALPHA_FACTOR_NTEX_ALPHA)
#define ALPHA_COMB_ADD			(R128_COMB_ALPHA_ADD |		\
					 R128_ALPHA_FACTOR_TEX_ALPHA)

#define INPUT_INTERP			(R128_INPUT_FACTOR_INT_COLOR |	\
					 R128_INP_FACTOR_A_INT_ALPHA)
#define INPUT_PREVIOUS			(R128_INPUT_FACTOR_PREV_COLOR |	\
					 R128_INP_FACTOR_A_PREV_ALPHA)

static void r128UpdateTextureEnv( GLcontext *ctx, int unit )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   GLint source = rmesa->tmu_source[unit];
   struct gl_texture_object *tObj;
   GLuint enabled;
   GLuint combine;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %d )\n",
	       __FUNCTION__, ctx, unit );
   }

   enabled = (ctx->Texture.ReallyEnabled >> (source * 4)) & TEXTURE0_ANY;
   if ( enabled != TEXTURE0_2D && enabled != TEXTURE0_1D )
      return;

   /* Only update the hardware texture state if the texture is current,
    * complete and enabled.
    */
   tObj = ctx->Texture.Unit[source].Current;
   if ( !tObj || !tObj->Complete )
      return;

   if ( ( tObj != ctx->Texture.Unit[source].CurrentD[2] ) &&
	( tObj != ctx->Texture.Unit[source].CurrentD[1] ) )
      return;

   if ( unit == 0 ) {
      combine = INPUT_INTERP;
   } else {
      combine = INPUT_PREVIOUS;
   }

   /* Set the texture environment state */
   switch ( ctx->Texture.Unit[source].EnvMode ) {
   case GL_REPLACE:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 combine |= (COLOR_COMB_DISABLE |		/* C = Ct            */
		     ALPHA_COMB_DISABLE);		/* A = At            */
	 break;
      case GL_RGB:
      case GL_LUMINANCE:
	 combine |= (COLOR_COMB_DISABLE |		/* C = Ct            */
		     ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	 break;
      case GL_ALPHA:
	 combine |= (COLOR_COMB_COPY_INPUT |		/* C = Cf            */
		     ALPHA_COMB_DISABLE);		/* A = At            */
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_MODULATE:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 combine |= (COLOR_COMB_MODULATE |		/* C = CfCt          */
		     ALPHA_COMB_MODULATE);		/* A = AfAt          */
	 break;
      case GL_RGB:
      case GL_LUMINANCE:
	 combine |= (COLOR_COMB_MODULATE |		/* C = CfCt          */
		     ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	 break;
      case GL_ALPHA:
	 combine |= (COLOR_COMB_COPY_INPUT |		/* C = Cf            */
		     ALPHA_COMB_MODULATE);		/* A = AfAt          */
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_DECAL:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
	 combine |= (COLOR_COMB_BLEND_TEX |		/* C = Cf(1-At)+CtAt */
		     ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	 break;
      case GL_RGB:
	 combine |= (COLOR_COMB_DISABLE |		/* C = Ct            */
		     ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	 break;
      case GL_ALPHA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 /* Undefined behaviour - just copy the incoming fragment */
	 combine |= (COLOR_COMB_COPY_INPUT |		/* C = undefined     */
		     ALPHA_COMB_COPY_INPUT);		/* A = undefined     */
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_BLEND:
      /* Rage 128 Pro and M3 can handle GL_BLEND texturing.
       */
      if ( !R128_IS_PLAIN( rmesa ) ) {
	 switch ( tObj->Image[0]->Format ) {
	 case GL_RGBA:
	 case GL_LUMINANCE_ALPHA:
	    combine |= (COLOR_COMB_BLEND_COLOR |	/* C = Cf(1-Ct)+CcCt */
			ALPHA_COMB_MODULATE);		/* A = AfAt          */
	    break;

	 case GL_RGB:
	 case GL_LUMINANCE:
	    combine |= (COLOR_COMB_BLEND_COLOR |	/* C = Cf(1-Ct)+CcCt */
			ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	    break;

	 case GL_ALPHA:
	    combine |= (COLOR_COMB_COPY_INPUT |		/* C = Cf            */
			ALPHA_COMB_MODULATE);		/* A = AfAt          */
	    break;

	 case GL_INTENSITY:
	    /* GH: We could be smarter about this... */
	    switch ( rmesa->env_color & 0xff000000 ) {
	    case 0x00000000:
	       combine |= (COLOR_COMB_BLEND_COLOR |	/* C = Cf(1-It)+CcIt */
			   ALPHA_COMB_MODULATE_NTEX);	/* A = Af(1-It)      */
	    default:
	       combine |= (COLOR_COMB_MODULATE |	/* C = fallback      */
			   ALPHA_COMB_MODULATE);	/* A = fallback      */
	       rmesa->Fallback |= R128_FALLBACK_TEXTURE;
	       break;
	    }
	    break;

	 case GL_COLOR_INDEX:
	 default:
	    return;
	 }
	 break;
      }

      /* Rage 128 has to fake some cases of GL_BLEND, otherwise fallback
       * to software rendering.
       */
      if ( rmesa->blend_flags ) {
	 rmesa->Fallback |= R128_FALLBACK_TEXTURE;
      }
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_LUMINANCE_ALPHA:
	 switch ( rmesa->env_color & 0x00ffffff ) {
	 case 0x00000000:
	    combine |= (COLOR_COMB_MODULATE_NTEX |	/* C = Cf(1-Ct)      */
			ALPHA_COMB_MODULATE);		/* A = AfAt          */
	    break;
	 case 0x00ffffff:
	    if ( unit == 0 ) {
	       combine |= (COLOR_COMB_MODULATE_NTEX |	/* C = Cf(1-Ct)      */
			   ALPHA_COMB_MODULATE);	/* A = AfAt          */
	    } else {
	       combine |= (COLOR_COMB_ADD |		/* C = Cf+Ct         */
			   ALPHA_COMB_COPY_INPUT);	/* A = Af            */
	    }
	    break;
	 default:
	    combine |= (COLOR_COMB_MODULATE |		/* C = fallback      */
			ALPHA_COMB_MODULATE);		/* A = fallback      */
	    rmesa->Fallback |= R128_FALLBACK_TEXTURE;
	    break;
	 }
	 break;
      case GL_RGB:
      case GL_LUMINANCE:
	 switch ( rmesa->env_color & 0x00ffffff ) {
	 case 0x00000000:
	    combine |= (COLOR_COMB_MODULATE_NTEX |	/* C = Cf(1-Ct)      */
			ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	    break;
	 case 0x00ffffff:
	    if ( unit == 0 ) {
	       combine |= (COLOR_COMB_MODULATE_NTEX |	/* C = Cf(1-Ct)      */
			   ALPHA_COMB_COPY_INPUT);	/* A = Af            */
	    } else {
	       combine |= (COLOR_COMB_ADD |		/* C = Cf+Ct         */
			   ALPHA_COMB_COPY_INPUT);	/* A = Af            */
	    }
	    break;
	 default:
	    combine |= (COLOR_COMB_MODULATE |		/* C = fallback      */
			ALPHA_COMB_COPY_INPUT);		/* A = fallback      */
	    rmesa->Fallback |= R128_FALLBACK_TEXTURE;
	    break;
	 }
	 break;
      case GL_ALPHA:
	 if ( unit == 0 ) {
	    combine |= (COLOR_COMB_COPY_INPUT |		/* C = Cf            */
			ALPHA_COMB_MODULATE);		/* A = AfAt          */
	 } else {
	    combine |= (COLOR_COMB_COPY_INPUT |		/* C = Cf            */
			ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	 }
	 break;
      case GL_INTENSITY:
	 switch ( rmesa->env_color & 0x00ffffff ) {
	 case 0x00000000:
	    combine |= COLOR_COMB_MODULATE_NTEX;	/* C = Cf(1-It)      */
	    break;
	 case 0x00ffffff:
	    if ( unit == 0 ) {
	       combine |= COLOR_COMB_MODULATE_NTEX;	/* C = Cf(1-It)      */
	    } else {
	       combine |= COLOR_COMB_ADD;		/* C = Cf+It         */
	    }
	    break;
	 default:
	    combine |= (COLOR_COMB_MODULATE |		/* C = fallback      */
			ALPHA_COMB_MODULATE);		/* A = fallback      */
	    rmesa->Fallback |= R128_FALLBACK_TEXTURE;
	    break;
	 }
	 switch ( rmesa->env_color & 0xff000000 ) {
	 case 0x00000000:
	    combine |= ALPHA_COMB_MODULATE_NTEX;	/* A = Af(1-It)      */
	    break;
	 case 0xff000000:
	    if ( unit == 0 ) {
	       combine |= ALPHA_COMB_MODULATE_NTEX;	/* A = Af(1-It)      */
	    } else {
	       combine |= ALPHA_COMB_ADD;		/* A = Af+It         */
	    }
	    break;
	 default:
	    combine |= (COLOR_COMB_MODULATE |		/* C = fallback      */
			ALPHA_COMB_MODULATE);		/* A = fallback      */
	    rmesa->Fallback |= R128_FALLBACK_TEXTURE;
	    break;
	 }
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_ADD:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_LUMINANCE_ALPHA:
	 combine |= (COLOR_COMB_ADD |			/* C = Cf+Ct         */
		     ALPHA_COMB_MODULATE);		/* A = AfAt          */
	 break;
      case GL_RGB:
      case GL_LUMINANCE:
	 combine |= (COLOR_COMB_ADD |			/* C = Cf+Ct         */
		     ALPHA_COMB_COPY_INPUT);		/* A = Af            */
	 break;
      case GL_ALPHA:
	 combine |= (COLOR_COMB_COPY_INPUT |		/* C = Cf            */
		     ALPHA_COMB_MODULATE);		/* A = AfAt          */
	 break;
      case GL_INTENSITY:
	 combine |= (COLOR_COMB_ADD |			/* C = Cf+Ct         */
		     ALPHA_COMB_ADD);			/* A = Af+At         */
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   default:
      return;
   }

   rmesa->tex_combine[unit] = combine;
}

static void r128UpdateTextureObject( GLcontext *ctx, int unit )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   int source = rmesa->tmu_source[unit];
   struct gl_texture_object *tObj;
   r128TexObjPtr t;
   GLuint enabled;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %d ) really=0x%x\n",
	       __FUNCTION__, ctx, unit, ctx->Texture.ReallyEnabled );
   }

   enabled = (ctx->Texture.ReallyEnabled >> (source * 4)) & TEXTURE0_ANY;
   if ( enabled != TEXTURE0_2D && enabled != TEXTURE0_1D ) {
      if ( enabled )
	 rmesa->Fallback |= R128_FALLBACK_TEXTURE;
      return;
   }

   /* Only update the hardware texture state if the texture is current,
    * complete and enabled.
    */
   tObj = ctx->Texture.Unit[source].Current;
   if ( !tObj || !tObj->Complete ) {
      rmesa->Fallback |= R128_FALLBACK_TEXTURE;
      return;
   }

   if ( ( tObj != ctx->Texture.Unit[source].CurrentD[2] ) &&
	( tObj != ctx->Texture.Unit[source].CurrentD[1] ) ) {
      rmesa->Fallback |= R128_FALLBACK_TEXTURE;
      return;
   }

   if ( !tObj->DriverData ) {
      /* If this is the first time the texture has been used, then create
       * a new texture object for it.
       */
      r128CreateTexObj( rmesa, tObj );

      if ( !tObj->DriverData ) {
	 /* Can't create a texture object... */
	 fprintf( stderr, "%s: texture object creation failed!\n",
		  __FUNCTION__ );
	 rmesa->Fallback |= R128_FALLBACK_TEXTURE;
	 return;
      }
   }

   /* We definately have a valid texture now */
   t = tObj->DriverData;

   /* Force the texture unit state to be loaded into the hardware */
   rmesa->dirty |= R128_UPLOAD_CONTEXT | (R128_UPLOAD_TEX0 << unit);

   /* Force any texture images to be loaded into the hardware */
   if ( t->dirty_images )
      rmesa->dirty |= (R128_UPLOAD_TEX0IMAGES << unit);

   /* Bind to the given texture unit */
   rmesa->CurrentTexObj[unit] = t;
   t->bound |= unit + 1;

   if ( t->memBlock )
      r128UpdateTexLRU( rmesa, t );

   if ( unit == 0 ) {
      rmesa->setup.tex_cntl_c       |= R128_TEXMAP_ENABLE;
      rmesa->setup.tex_size_pitch_c |= t->setup.tex_size_pitch << 0;
      rmesa->setup.scale_3d_cntl    &= ~R128_TEX_CACHE_SPLIT;

      t->setup.tex_cntl &= ~R128_SEC_SELECT_SEC_ST;
   } else {
      t->setup.tex_cntl |=  R128_SEC_SELECT_SEC_ST;

      rmesa->setup.tex_cntl_c       |= R128_SEC_TEXMAP_ENABLE;
      rmesa->setup.tex_size_pitch_c |= t->setup.tex_size_pitch << 16;
      rmesa->setup.scale_3d_cntl    |= R128_TEX_CACHE_SPLIT;
   }
}

void r128UpdateTextureState( GLcontext *ctx )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p ) en=0x%x\n",
	       __FUNCTION__, ctx, ctx->Texture.ReallyEnabled );
   }

   /* Clear any texturing fallbacks */
   rmesa->Fallback &= ~R128_FALLBACK_TEXTURE;

   /* Unbind any currently bound textures */
   if ( rmesa->CurrentTexObj[0] ) rmesa->CurrentTexObj[0]->bound = 0;
   if ( rmesa->CurrentTexObj[1] ) rmesa->CurrentTexObj[1]->bound = 0;
   rmesa->CurrentTexObj[0] = NULL;
   rmesa->CurrentTexObj[1] = NULL;

   if ( ctx->Enabled & (TEXTURE0_3D|TEXTURE1_3D) )
      rmesa->Fallback |= R128_FALLBACK_TEXTURE;

   /* Disable all texturing until it is known to be good */
   rmesa->setup.tex_cntl_c &= ~(R128_TEXMAP_ENABLE |
				R128_SEC_TEXMAP_ENABLE);
   rmesa->setup.tex_size_pitch_c = 0x00000000;

   r128UpdateTextureObject( ctx, 0 );
   r128UpdateTextureEnv( ctx, 0 );

   if ( rmesa->multitex ) {
      r128UpdateTextureObject( ctx, 1 );
      r128UpdateTextureEnv( ctx, 1 );
   }

   rmesa->dirty |= R128_UPLOAD_CONTEXT;
}


/* ================================================================
 * DD interface texturing functions
 *
 * FIXME: Many of these are deprecated -- we should move to the new
 * single-copy texture interface.
 */

static void r128DDTexEnv( GLcontext *ctx, GLenum target,
			  GLenum pname, const GLfloat *param )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   struct gl_texture_unit *texUnit;
   GLubyte c[4];

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %s )\n",
	       __FUNCTION__, gl_lookup_enum_by_nr( pname ) );
   }

   switch ( pname ) {
   case GL_TEXTURE_ENV_MODE:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= R128_NEW_TEXTURE | R128_NEW_ALPHA;
      break;

   case GL_TEXTURE_ENV_COLOR:
      texUnit = &ctx->Texture.Unit[ctx->Texture.CurrentUnit];
      FLOAT_RGBA_TO_UBYTE_RGBA( c, texUnit->EnvColor );
      rmesa->env_color = r128PackColor( 4, c[0], c[1], c[2], c[3] );
      if ( rmesa->setup.constant_color_c != rmesa->env_color ) {
	 FLUSH_BATCH( rmesa );
	 rmesa->setup.constant_color_c = rmesa->env_color;

	 rmesa->new_state |= R128_NEW_TEXTURE;

	 /* More complex multitexture/multipass fallbacks for GL_BLEND
	  * can be done later, but this allows a single pass GL_BLEND
	  * in some cases (ie. Performer town demo).  This is only
	  * applicable to the regular Rage 128, as the Pro and M3 can
	  * handle true single-pass GL_BLEND texturing.
	  */
	 rmesa->blend_flags &= ~R128_BLEND_ENV_COLOR;
	 if ( R128_IS_PLAIN( rmesa ) &&
	      rmesa->env_color != 0x00000000 &&
	      rmesa->env_color != 0xff000000 &&
	      rmesa->env_color != 0x00ffffff &&
	      rmesa->env_color != 0xffffffff ) {
	    rmesa->blend_flags |= R128_BLEND_ENV_COLOR;
	 }
      }
      break;

   case GL_TEXTURE_LOD_BIAS_EXT:
      do {
	 CARD32 t = rmesa->setup.tex_cntl_c;
	 GLint bias;
	 CARD32 b;

	 /* GTH: This isn't exactly correct, but gives good results up to a
	  * certain point.  It is better than completely ignoring the LOD
	  * bias.  Unfortunately there isn't much range in the bias, the
	  * spec mentions strides that vary between 0.5 and 2.0 but these
	  * numbers don't seem to relate the the GL LOD bias value at all.
	  */
	 if ( param[0] >= 1.0 ) {
	    bias = -128;
	 } else if ( param[0] >= 0.5 ) {
	    bias = -64;
	 } else if ( param[0] >= 0.25 ) {
	    bias = 0;
	 } else if ( param[0] >= 0.0 ) {
	    bias = 63;
	 } else {
	    bias = 127;
	 }

	 b = (CARD32)bias & 0xff;
	 t &= ~R128_LOD_BIAS_MASK;
	 t |= (b << R128_LOD_BIAS_SHIFT);

	 if ( rmesa->setup.tex_cntl_c != t ) {
	    FLUSH_BATCH( rmesa );
	    rmesa->setup.tex_cntl_c = t;
	    rmesa->dirty |= R128_UPLOAD_CONTEXT;
	 }
      } while (0);
      break;

   default:
      return;
   }
}

static void r128DDTexImage( GLcontext *ctx, GLenum target,
			    struct gl_texture_object *tObj, GLint level,
			    GLint internalFormat,
			    const struct gl_texture_image *image )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   r128TexObjPtr t;

   if ( R128_DEBUG & DEBUG_VERBOSE_API )
      fprintf( stderr, "%s( %p, level %d )\n", __FUNCTION__, tObj, level );

   if ( ( target != GL_TEXTURE_2D ) &&
	( target != GL_TEXTURE_1D ) )
      return;

   if ( level >= R128_MAX_TEXTURE_LEVELS )
      return;

   t = (r128TexObjPtr)tObj->DriverData;
   if ( t ) {
      if ( t->bound ) FLUSH_BATCH( rmesa );

      /* Destroy the old texture, and upload a new one.  The actual
       * uploading of the texture image occurs in the UploadSubImage
       * function.
       */
      r128DestroyTexObj( rmesa, t );
      rmesa->new_state |= R128_NEW_TEXTURE;
   }
}

static void r128DDTexSubImage( GLcontext *ctx, GLenum target,
			       struct gl_texture_object *tObj, GLint level,
			       GLint xoffset, GLint yoffset,
			       GLsizei width, GLsizei height,
			       GLint internalFormat,
			       const struct gl_texture_image *image )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   r128TexObjPtr t;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, level %d ) size: %d,%d of %d,%d\n",
	       __FUNCTION__, tObj, level, width, height,
	       image->Width, image->Height );
   }

   if ( ( target != GL_TEXTURE_2D ) &&
	( target != GL_TEXTURE_1D ) )
      return;

   if ( level >= R128_MAX_TEXTURE_LEVELS )
      return;

   t = (r128TexObjPtr)tObj->DriverData;
   if ( t ) {
      if ( t->bound ) FLUSH_BATCH( rmesa );

#if 0
      /* FIXME: Only upload textures if we already have space in the heap.
       */
      LOCK_HARDWARE( rmesa );
      r128UploadSubImage( rmesa, t, level,
			  xoffset, yoffset, width, height );
      UNLOCK_HARDWARE( rmesa );

      /* Update the context state */
      rmesa->setup.tex_cntl_c |= R128_TEX_CACHE_FLUSH;
#else
      r128DestroyTexObj( rmesa, t );
#endif

      rmesa->new_state |= R128_NEW_TEXTURE;
   }
}

static void r128DDTexParameter( GLcontext *ctx, GLenum target,
				struct gl_texture_object *tObj,
				GLenum pname, const GLfloat *params )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   r128TexObjPtr t = (r128TexObjPtr)tObj->DriverData;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %s )\n",
	       __FUNCTION__, gl_lookup_enum_by_nr( pname ) );
   }

   /* If we don't have a hardware texture, it will be automatically
    * created with current state before it is used, so we don't have
    * to do anything now.
    */
   if ( !t || !t->bound )
      return;

   if ( ( target != GL_TEXTURE_2D ) &&
	( target != GL_TEXTURE_1D ) )
      return;

   switch ( pname ) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
      if ( t->bound ) FLUSH_BATCH( rmesa );
      r128SetTexFilter( t, tObj->MinFilter, tObj->MagFilter );
      break;

   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
      if ( t->bound ) FLUSH_BATCH( rmesa );
      r128SetTexWrap( t, tObj->WrapS, tObj->WrapT );
      break;

   case GL_TEXTURE_BORDER_COLOR:
      if ( t->bound ) FLUSH_BATCH( rmesa );
      r128SetTexBorderColor( t, tObj->BorderColor );
      break;

   default:
      return;
   }

   rmesa->new_state |= R128_NEW_TEXTURE;
}

static void r128DDBindTexture( GLcontext *ctx, GLenum target,
			       struct gl_texture_object *tObj )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   GLint unit = ctx->Texture.CurrentUnit;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p ) unit=%d\n",
	       __FUNCTION__, tObj, unit );
   }

   FLUSH_BATCH( rmesa );

   if ( rmesa->CurrentTexObj[unit] ) {
      rmesa->CurrentTexObj[unit]->bound &= ~(unit+1);
      rmesa->CurrentTexObj[unit] = NULL;
   }

   rmesa->new_state |= R128_NEW_TEXTURE;
}

static void r128DDDeleteTexture( GLcontext *ctx,
				 struct gl_texture_object *tObj )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   r128TexObjPtr t = (r128TexObjPtr)tObj->DriverData;

   if ( t ) {
      if ( t->bound ) {
	 FLUSH_BATCH( rmesa );

	 if ( t->bound & TEX_0 ) rmesa->CurrentTexObj[0] = NULL;
	 if ( t->bound & TEX_1 ) rmesa->CurrentTexObj[1] = NULL;
	 rmesa->new_state |= R128_NEW_TEXTURE;
      }

      r128DestroyTexObj( rmesa, t );
      tObj->DriverData = NULL;
   }
}

static GLboolean r128DDIsTextureResident( GLcontext *ctx,
					  struct gl_texture_object *tObj )
{
   r128TexObjPtr t = (r128TexObjPtr)tObj->DriverData;

   return ( t && t->memBlock );
}



void r128DDInitTextureFuncs( GLcontext *ctx )
{
   ctx->Driver.TexEnv			= r128DDTexEnv;
   ctx->Driver.TexImage			= r128DDTexImage;
   ctx->Driver.TexSubImage		= r128DDTexSubImage;
   ctx->Driver.TexParameter		= r128DDTexParameter;
   ctx->Driver.BindTexture		= r128DDBindTexture;
   ctx->Driver.DeleteTexture		= r128DDDeleteTexture;
   ctx->Driver.UpdateTexturePalette	= NULL;
   ctx->Driver.ActiveTexture		= NULL;
   ctx->Driver.IsTextureResident	= r128DDIsTextureResident;
   ctx->Driver.PrioritizeTexture	= NULL;
}
