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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_tex.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>

#include "types.h"
#include "enums.h"
#include "pb.h"
#include "dd.h"

#include "mm.h"
#include "enums.h"

#include "i830_drv.h"
#include "i830_ioctl.h"
#include "simple_list.h"
#include "texutil.h"

static void i830SetTexWrapping(i830TextureObjectPtr tex,
			       GLenum swrap, GLenum twrap)
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   tex->Setup[I830_TEXREG_MCS] &= ~(TEXCOORD_ADDR_U_MASK|TEXCOORD_ADDR_V_MASK);

   switch( swrap ) {
   case GL_REPEAT:
      tex->Setup[I830_TEXREG_MCS] |= TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_WRAP);
      break;
   case GL_CLAMP:
      tex->Setup[I830_TEXREG_MCS] |= TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_CLAMP);
      break;
   case GL_CLAMP_TO_EDGE:
      tex->Setup[I830_TEXREG_MCS] |= 
			TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_CLAMP_BORDER);
      break;
   }
   switch( twrap ) {
   case GL_REPEAT:
      tex->Setup[I830_TEXREG_MCS] |= TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_WRAP);
      break;
   case GL_CLAMP:
      tex->Setup[I830_TEXREG_MCS] |= TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_CLAMP);
      break;
   case GL_CLAMP_TO_EDGE:
      tex->Setup[I830_TEXREG_MCS] |= 
			TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_CLAMP_BORDER);
      break;
   }

}

static void i830SetTexFilter(i830ContextPtr imesa, 
			     i830TextureObjectPtr t, 
			     GLenum minf, GLenum magf)
{
   GLuint LastLevel;
   int minFilt = 0, mipFilt = 0, magFilt = 0;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   switch (minf) {
   case GL_NEAREST:
      minFilt = FILTER_NEAREST;
      mipFilt = MIPFILTER_NONE;
      break;
   case GL_LINEAR:
      minFilt = FILTER_LINEAR;
      mipFilt = MIPFILTER_NONE;
      break;
   case GL_NEAREST_MIPMAP_NEAREST:
      minFilt = FILTER_NEAREST;
      mipFilt = MIPFILTER_NEAREST;
      break;
   case GL_LINEAR_MIPMAP_NEAREST:
      minFilt = FILTER_LINEAR;
      mipFilt = MIPFILTER_NEAREST;
      break;
   case GL_NEAREST_MIPMAP_LINEAR:
      minFilt = FILTER_NEAREST;
      mipFilt = MIPFILTER_LINEAR;
      break;
   case GL_LINEAR_MIPMAP_LINEAR:
      minFilt = FILTER_LINEAR;
      mipFilt = MIPFILTER_LINEAR;
      break;
   default:
      fprintf(stderr, "i830SetTexFilter(): not supported min. filter %d\n",
	      (int)minf);
      break;
   }

   I830_SET_FIELD(t->Setup[I830_TEXREG_MF],
		     MIN_FILTER_MASK | MIP_FILTER_MASK,
		     MIN_FILTER(minFilt) | mipFilt);

   switch (magf) {
   case GL_NEAREST:
      magFilt = FILTER_NEAREST;
      break;
   case GL_LINEAR:
      magFilt = FILTER_LINEAR;
      break;
   default:
      fprintf(stderr, "i830SetTexFilter(): not supported mag. filter %d\n",
	      (int)magf);
      break;
   }  

   I830_SET_FIELD(t->Setup[I830_TEXREG_MF],
		     MAG_FILTER_MASK, MAG_FILTER(magFilt));

   if (t->globj->MinFilter != GL_NEAREST && 
       t->globj->MinFilter != GL_LINEAR) {
      LastLevel = t->max_level;
   } else {
      LastLevel = t->min_level;
   }

   I830_SET_FIELD(t->Setup[I830_TEXREG_MLL],
		  LOD_MAX_MASK,
		  LOD_MAX(t->min_level << 4));

   I830_SET_FIELD(t->Setup[I830_TEXREG_MLL],
		  LOD_MIN_MASK,
		  LOD_MIN(LastLevel));

   /* See OpenGL 1.2 specification */
   if (magf == GL_LINEAR && (minf == GL_NEAREST_MIPMAP_NEAREST || 
			     minf == GL_NEAREST_MIPMAP_LINEAR))
   {
      /* c = 0.5 */
      I830_SET_FIELD(t->Setup[I830_TEXREG_MLC],
		     MAP_LOD_MASK, 0x10);
   } else {
      /* c = 0 */
      I830_SET_FIELD(t->Setup[I830_TEXREG_MLC],
		     MAP_LOD_MASK, 0x0);
   }
}


/* XXX - have to make sure MI0 has length field set to include color */
static void i830SetTexBorderColor(i830TextureObjectPtr t, GLubyte color[4])
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

    t->Setup[I830_TEXREG_MI5] = 
        I830PACKCOLOR8888(color[0],color[1],color[2],color[3]);
}


static void ReplicateMesaTexState(i830ContextPtr imesa,
				  i830TextureObjectPtr t,
                                  struct gl_texture_object *mesatex)
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   i830SetTexWrapping(t,mesatex->WrapS,mesatex->WrapT);
   i830SetTexFilter(imesa, t,mesatex->MinFilter,mesatex->MagFilter);
   i830SetTexBorderColor(t,mesatex->BorderColor);
}

/* Utility function to setup the texture palette */
static void
i830ConvertPalette(GLuint *data, const struct gl_color_table *table)
{
   const GLubyte *tableUB = (const GLubyte *) table->Table;
   GLint width = table->Size;
   GLuint r, g, b, a;
   int i;

   ASSERT(table->TableType == GL_UNSIGNED_BYTE);

   switch(table->Format) {
   case GL_RGBA:
      for(i = 0; i < width; i++) {
	 r = tableUB[i * 4 + 0];
	 g = tableUB[i * 4 + 1];
	 b = tableUB[i * 4 + 2];
	 a = tableUB[i * 4 + 3];
	 data[i] = I830PACKCOLOR4444(r, g, b, a);
      }
      break;
   case GL_RGB:
      for (i = 0; i < width; i++) {
	 r = tableUB[i * 3 + 0];
	 g = tableUB[i * 3 + 1];
	 b = tableUB[i * 3 + 2];
	 data[i] = I830PACKCOLOR565(r, g, b);
      }
      break;
   case GL_LUMINANCE:
      for (i = 0; i < width; i++) {
	 r = tableUB[i];
	 data[i] = (255 << 8) | r;
      }
      break;
   case GL_ALPHA:
      for (i = 0; i < width; i++) {
	 a = tableUB[i];
	 data[i] = (a << 8) | 255;
      }
      break;
   case GL_LUMINANCE_ALPHA:
      for (i = 0; i < width; i++) {
	 r = tableUB[i * 2 + 0];
	 a = tableUB[i * 2 + 1];
	 data[i] = (a << 8) | r;
      }
      break;
   case GL_INTENSITY:
      for (i = 0; i < width; i++) {
	 a = tableUB[i];
	 data[i] = (a << 8) | a;
      }
      break;
   }
}

static i830TextureObjectPtr i830CreateTexObj(i830ContextPtr imesa,
					     struct gl_texture_object *tObj)
{
   i830TextureObjectPtr t;
   GLuint height, width, pitch, i, textureFormat;
   struct gl_texture_image *image;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   image = tObj->Image[ 0 ];
   if ( !image ) {
      fprintf(stderr, "no image at level zero - not creating texobj\n");
      return 0;
   }
   
   t = (i830TextureObjectPtr) calloc(1,sizeof(*t));
   if (!t) {
      fprintf(stderr, "failed to allocate memory - not creating texobj\n");
      return 0;
   }

   switch( image->Format ) {
   case GL_RGB:
      image->TexFormat = &(_mesa_texformat_rgb565);
      t->texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_RGB565;
      break;
   case GL_RGBA:
      image->TexFormat = &(_mesa_texformat_argb4444);
      t->texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_ARGB4444;
      break;
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
   case GL_INTENSITY:
      image->TexFormat = &(_mesa_texformat_al88);
      t->texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_AY88;
      break;
   case GL_COLOR_INDEX:
      image->TexFormat = &(_mesa_texformat_ci8);
      textureFormat = MAPSURF_8BIT_INDEXED;
      t->texelBytes = 1;

      switch(tObj->Palette.Format) {
      case GL_RGBA: 
	 textureFormat |= MT_8BIT_IDX_ARGB4444;
	 break;
      case GL_RGB: 
	 textureFormat |= MT_8BIT_IDX_RGB565;
	 break;
      case GL_LUMINANCE: 
      case GL_ALPHA:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY: 
	 textureFormat |= MT_8BIT_IDX_AY88; break;
      }

      /* Insure the palette is loaded */
      i830ConvertPalette(t->palette, &tObj->Palette);
      t->palette_format = tObj->Palette.Format;
      break;
   default:
      fprintf(stderr, "i830CreateTexObj: bad image->Format\n");
      free( t );      
      return 0;	
   }

   /* Figure out the size now (and count the levels).  Upload won't be done
    * until later.
    */ 
   width = image->Width * t->texelBytes;
   if(width % 4) {
      fprintf(stderr, "Pitch is not a multiple of dwords\n");
   }
   pitch = width; /* All pitches can be used, since we are not using 
		   * tiled surfaces.
		   */

   t->dirty_images = 0;
   t->bound = 0;
   
   for ( height = i = 0 ; i < I830_TEX_MAXLEVELS && tObj->Image[i] ; i++ ) {
      t->image[i].image = tObj->Image[i];
      t->image[i].offset = height * pitch;
      t->image[i].internalFormat = image->Format;
      t->dirty_images |= (1<<i);
      height += t->image[i].image->Height;
   }

   t->Pitch = pitch;
   t->totalSize = height*pitch;
   t->max_level = i-1;
   t->min_level = 0;
   t->globj = tObj;
   t->age = 0;

   t->Setup[I830_TEXREG_MI0] = STATE3D_MAP_INFO_COLR_CMD;

   t->Setup[I830_TEXREG_MI1] = MAP_INFO_TEX(0) |
				textureFormat |
				MAP_INFO_OUTMUX_F0F1F2F3 | 
				MAP_INFO_VERTLINESTRIDE_0 |
				MAP_INFO_VERTLINESTRIDEOFS_0 |
				MAP_INFO_FORMAT_2D |
				MAP_INFO_USE_FENCE;

   t->Setup[I830_TEXREG_MI2] = (((1 << image->HeightLog2) - 1) << 16) |
				((1 << image->WidthLog2) - 1);

   t->Setup[I830_TEXREG_MI3] = 0;

   t->Setup[I830_TEXREG_MI4] = ((pitch / 4) - 1) << 2;

   t->Setup[I830_TEXREG_MI5] = 0;
  
   t->Setup[I830_TEXREG_MLC] = STATE3D_MAP_LOD_CNTL_CMD | MAP_UNIT(0) | 
				ENABLE_TEXLOD_BIAS |
				MAP_LOD_BIAS(0);

   t->Setup[I830_TEXREG_MLL] = STATE3D_MAP_LOD_LIMITS_CMD | MAP_UNIT(0) |
				ENABLE_MAX_MIP_LVL | 
				LOD_MAX(t->min_level << 4) |
				ENABLE_MIN_MIP_LVL |
				LOD_MIN(t->max_level);

   /* I think this is context state, really.
    */
   t->Setup[I830_TEXREG_MCS] = STATE3D_MAP_COORD_SET_CMD | MAP_UNIT(0) |
				ENABLE_TEXCOORD_PARAMS |
				TEXCOORDS_ARE_NORMAL |
				TEXCOORDTYPE_CARTESIAN |
				ENABLE_ADDR_V_CNTL |
				TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_WRAP) |
				ENABLE_ADDR_U_CNTL |
				TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_WRAP);

   t->Setup[I830_TEXREG_MF] = STATE3D_MAP_FILTER_CMD | MAP_UNIT(0) |
			       ENABLE_MIP_MODE_FILTER |
			       MIPFILTER_NEAREST |
			       ENABLE_MAG_MODE_FILTER |
			       MAG_FILTER(FILTER_LINEAR) |
			       ENABLE_MIN_MODE_FILTER |
			       MIN_FILTER(FILTER_LINEAR);

   t->current_unit = 0;

   ReplicateMesaTexState(imesa, t,tObj);
   tObj->DriverData = t;
   /* Forces tex cache flush */
   imesa->dirty |= I830_UPLOAD_CTX;
   make_empty_list( t );
   return t;
}

void i830DestroyTexObj(i830ContextPtr imesa, i830TextureObjectPtr t)
{
   if (!t) return;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   /* This is sad - need to sync *in case* we upload a texture
    * to this newly free memory...
    */
   if (t->MemBlock) {
      mmFreeMem(t->MemBlock);
      t->MemBlock = 0;

      if (t->age > imesa->dirtyAge)
	 imesa->dirtyAge = t->age;
   }

   if (t->globj)
      t->globj->DriverData = 0;

   if (t->bound)
      imesa->CurrentTexObj[t->bound - 1] = 0; 

   remove_from_list(t);
   free(t);
}


static void i830SwapOutTexObj(i830ContextPtr imesa, i830TextureObjectPtr t)
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if (t->MemBlock) {
      mmFreeMem(t->MemBlock);
      t->MemBlock = 0;      

      if (t->age > imesa->dirtyAge)
	 imesa->dirtyAge = t->age;
   }

   t->dirty_images = ~0;
   move_to_tail(&(imesa->SwappedOut), t);
}



/* Upload an image from mesa's internal copy.
 */
static void i830UploadTexLevel( i830TextureObjectPtr t, int level )
{
   const struct gl_texture_image *image = t->image[level].image;
   int i,j;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if (I830_DEBUG & DEBUG_VERBOSE_LRU)
      fprintf(stderr, "i830UploadTexLevel %d, BufAddr %p offset %x\n",
	      level, t->BufAddr, t->image[level].offset);

   switch (t->image[level].internalFormat) {
   case GL_RGB:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = I830PACKCOLOR565(src[0],src[1],src[2]);
	    src += 3;
	 }
      }
   }
   break;
      
   case GL_RGBA:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = I830PACKCOLOR4444(src[0],src[1],src[2],src[3]);
	    src += 4;
	 }
      }
   }
   break;

   case GL_INTENSITY:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;
      int i;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = (src[0] << 8) | (src[0]);
	    src ++;
	 }
      }
   }
   break;

   case GL_LUMINANCE:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = (255 << 8) | (src[0]);
	    src ++;
	 }
      }
   }
   break;

   case GL_LUMINANCE_ALPHA:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = (src[1] << 8) | (src[0]);
	    src += 2;
	 }
      }
   }
   break;

   case GL_ALPHA:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = (src[0] << 8) | 255;
	    src += 1;
	 }
      }
   }
   break;

   case GL_COLOR_INDEX:
      {
	 GLubyte *dst = (GLubyte *)(t->BufAddr + t->image[level].offset);
	 GLubyte *src = (GLubyte *)image->Data;

	 for (j = 0 ; j < image->Height ; j++, dst += t->Pitch) {
	    for (i = 0 ; i < image->Width ; i++) {
	       dst[i] = src[0];
	       src += 1;
	    }
	 }
      }
   break;
      
   default:
      fprintf(stderr, "Not supported texture format %s\n",
	      gl_lookup_enum_by_nr(image->Format));
   }
}



void i830PrintLocalLRU( i830ContextPtr imesa ) 
{
   i830TextureObjectPtr t;
   int sz = 1 << (imesa->i830Screen->logTextureGranularity);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   foreach( t, &imesa->TexObjList ) {
      if (!t->globj)
	 fprintf(stderr, "Placeholder %d at %x sz %x\n", 
		 t->MemBlock->ofs / sz,
		 t->MemBlock->ofs,
		 t->MemBlock->size);      
      else
	 fprintf(stderr, "Texture (bound %d) at %x sz %x\n", 
		 t->bound,
		 t->MemBlock->ofs,
		 t->MemBlock->size);      

   }
}

void i830PrintGlobalLRU( i830ContextPtr imesa )
{
   int i, j;
   I830TexRegion *list = imesa->sarea->texList;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   for (i = 0, j = I830_NR_TEX_REGIONS ; i < I830_NR_TEX_REGIONS ; i++) {
      fprintf(stderr, "list[%d] age %d next %d prev %d\n",
	      j, list[j].age, list[j].next, list[j].prev);
      j = list[j].next;
      if (j == I830_NR_TEX_REGIONS) break;
   }
   
   if (j != I830_NR_TEX_REGIONS)
      fprintf(stderr, "Loop detected in global LRU\n");
}


void i830ResetGlobalLRU( i830ContextPtr imesa )
{
   I830TexRegion *list = imesa->sarea->texList;
   int sz = 1 << imesa->i830Screen->logTextureGranularity;
   int i;

   /* (Re)initialize the global circular LRU list.  The last element
    * in the array (I830_NR_TEX_REGIONS) is the sentinal.  Keeping it
    * at the end of the array allows it to be addressed rationally
    * when looking up objects at a particular location in texture
    * memory.  
    */

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   for (i = 0 ; (i+1) * sz <= imesa->i830Screen->textureSize ; i++) {
      list[i].prev = i-1;
      list[i].next = i+1;
      list[i].age = 0;
   }

   i--;
   list[0].prev = I830_NR_TEX_REGIONS;
   list[i].prev = i-1;
   list[i].next = I830_NR_TEX_REGIONS;
   list[I830_NR_TEX_REGIONS].prev = i;
   list[I830_NR_TEX_REGIONS].next = 0;
   imesa->sarea->texAge = 0;
}


static void i830UpdateTexLRU( i830ContextPtr imesa, i830TextureObjectPtr t ) 
{
   int i;
   int logsz = imesa->i830Screen->logTextureGranularity;
   int start = t->MemBlock->ofs >> logsz;
   int end = (t->MemBlock->ofs + t->MemBlock->size - 1) >> logsz;
   I830TexRegion *list = imesa->sarea->texList;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);
   
   imesa->texAge = ++imesa->sarea->texAge;

   /* Update our local LRU
    */
   move_to_head( &(imesa->TexObjList), t );

   /* Update the global LRU
    */
   for (i = start ; i <= end ; i++) {

      list[i].in_use = 1;
      list[i].age = imesa->texAge;

      /* remove_from_list(i)
       */
      list[(unsigned)list[i].next].prev = list[i].prev;
      list[(unsigned)list[i].prev].next = list[i].next;
      
      /* insert_at_head(list, i)
       */
      list[i].prev = I830_NR_TEX_REGIONS;
      list[i].next = list[I830_NR_TEX_REGIONS].next;
      list[(unsigned)list[I830_NR_TEX_REGIONS].next].prev = i;
      list[I830_NR_TEX_REGIONS].next = i;
   }
}


/* Called for every shared texture region which has increased in age
 * since we last held the lock.
 *
 * Figures out which of our textures have been ejected by other clients,
 * and pushes a placeholder texture onto the LRU list to represent 
 * the other client's textures.  
 */
void i830TexturesGone( i830ContextPtr imesa,
		       GLuint offset, 
		       GLuint size,
		       GLuint in_use ) 
{
   i830TextureObjectPtr t, tmp;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   foreach_s ( t, tmp, &imesa->TexObjList ) {

      if (t->MemBlock->ofs >= offset + size ||
	  t->MemBlock->ofs + t->MemBlock->size <= offset)
	 continue;

      /* It overlaps - kick it off.  Need to hold onto the currently bound
       * objects, however.
       */
      if (t->bound)
	 i830SwapOutTexObj( imesa, t );
      else
	 i830DestroyTexObj( imesa, t );
   }

   
   if (in_use) {
      t = (i830TextureObjectPtr) calloc(1,sizeof(*t));
      if (!t) return;

      t->MemBlock = mmAllocMem( imesa->texHeap, size, 0, offset);      
      insert_at_head( &imesa->TexObjList, t );
   }
}

/* This is called with the lock held.  May have to eject our own and/or
 * other client's texture objects to make room for the upload.
 */
int i830UploadTexImages( i830ContextPtr imesa, i830TextureObjectPtr t )
{
   int i;
   int ofs;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   /* Do we need to eject LRU texture objects?
    */
   if (!t->MemBlock) {
      while (1)
      {
	 t->MemBlock = mmAllocMem( imesa->texHeap, t->totalSize, 12, 0 ); 
	 if (t->MemBlock)
	    break;

	 if (imesa->TexObjList.prev->bound) {
  	    fprintf(stderr, "Hit bound texture in upload\n"); 
	    i830PrintLocalLRU( imesa );
	    return -1;
	 }

	 if (imesa->TexObjList.prev == &(imesa->TexObjList)) {
 	    fprintf(stderr, "Failed to upload texture, sz %d\n", t->totalSize);
	    mmDumpMemInfo( imesa->texHeap );
	    return -1;
	 }
	 
	 i830DestroyTexObj( imesa, imesa->TexObjList.prev );
      }
 
      ofs = t->MemBlock->ofs;
      t->Setup[I830_TEXREG_MI3] = imesa->i830Screen->textureOffset + ofs;
      t->BufAddr = imesa->i830Screen->tex.map + ofs;
      imesa->dirty |= I830_UPLOAD_CTX;
   }

   /* Let the world know we've used this memory recently.
    */
   i830UpdateTexLRU( imesa, t );

   if (I830_DEBUG & DEBUG_VERBOSE_LRU)
      fprintf(stderr, "dispatch age: %d age freed memory: %d\n",
	      GET_DISPATCH_AGE(imesa), imesa->dirtyAge);

   if (imesa->dirtyAge >= GET_DISPATCH_AGE(imesa)) 
      i830WaitAgeLocked( imesa, imesa->dirtyAge );
   

   if (t->dirty_images) {
      if (I830_DEBUG & DEBUG_VERBOSE_LRU)
	 fprintf(stderr, "*");

      for (i = t->min_level ; i <= t->max_level ; i++)
	 if (t->dirty_images & (1<<i)) 
	    i830UploadTexLevel( t, i );
   }


   t->dirty_images = 0;
   return 0;
}

static void i830TexSetUnit( i830TextureObjectPtr t, GLuint unit )
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s unit(%d)\n", __FUNCTION__, unit);

   /* This will need to be changed when I support more then 2 t units */
   I830_SET_FIELD(t->Setup[I830_TEXREG_MI1],
		  MAP_INFO_MASK | MAP_INFO_USE_PALETTE_1,
		  MAP_INFO_TEX(unit) | MAP_INFO_USE_PALETTE_N(unit));
   I830_SET_FIELD(t->Setup[I830_TEXREG_MLC], MAP_UNIT_MASK, MAP_UNIT(unit));
   I830_SET_FIELD(t->Setup[I830_TEXREG_MLL], MAP_UNIT_MASK, MAP_UNIT(unit));
   I830_SET_FIELD(t->Setup[I830_TEXREG_MCS], MAP_UNIT_MASK, MAP_UNIT(unit));
   I830_SET_FIELD(t->Setup[I830_TEXREG_MF], MAP_UNIT_MASK, MAP_UNIT(unit));

   t->current_unit = unit;
}

static __inline__ GLuint GetTexelOp(GLint unit)
{
   switch(unit) {
    case 0: return TEXBLENDARG_TEXEL0;
    case 1: return TEXBLENDARG_TEXEL1;
    case 2: return TEXBLENDARG_TEXEL2;
    case 3: return TEXBLENDARG_TEXEL3;
    default: return TEXBLENDARG_TEXEL0;
   }
}

/* Only 1.2 modes; make another func for combine, combine4, combiners */
static void i830SetBlend_GL1_2(i830ContextPtr imesa, int curTex, 
			       GLenum envMode, GLenum format)
{
   GLuint texel_op = GetTexelOp(curTex);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
     fprintf(stderr, "%s %s %s unit (%d) texel_op(0x%x)\n",
	     __FUNCTION__,
	     gl_lookup_enum_by_nr(format),
	     gl_lookup_enum_by_nr(envMode),
	     curTex,
	     texel_op);

   switch(envMode) {
   case GL_REPLACE:
      switch(format) {
      case GL_ALPHA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      case GL_LUMINANCE:
      case GL_RGB:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;

      case GL_INTENSITY:
      case GL_LUMINANCE_ALPHA:
      case GL_RGBA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      default:
	 /* Always set to passthru if something is funny */
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      }
      break;

   case GL_MODULATE:
      switch(format) {
      case GL_ALPHA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 5;
	 break;

      case GL_LUMINANCE:
      case GL_RGB:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 5;
	 break;

      case GL_INTENSITY:
      case GL_LUMINANCE_ALPHA:
      case GL_RGBA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 6;
	 break;
      default:
	 /* Always set to passthru if something is funny */
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      }
      break;

   case GL_DECAL:
      switch(format) {
      case GL_RGB:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;

      case GL_RGBA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_BLEND);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG0 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_REPLICATE_ALPHA |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 6;
	 break;
      default:
	 /* Always set to passthru if something is funny */
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      }
      break;

   case GL_BLEND:
      switch(format) {
      case GL_ALPHA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 5;
	 break;

      case GL_LUMINANCE:
      case GL_RGB:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_BLEND);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG0 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_FACTOR_N);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 6;
	 break;

      case GL_INTENSITY:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_BLEND);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_BLEND);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG0 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_FACTOR_N);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG0 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][6] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_FACTOR_N);
	 imesa->TexBlend[curTex][7] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 8;
	 break;

      case GL_LUMINANCE_ALPHA:
      case GL_RGBA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_BLEND);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG0 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_FACTOR_N);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][6] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 7;
	 break;
      default:
	 /* Always set to passthru if something is funny */
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      }
      break;

   case GL_ADD:
      switch(format) {
      case GL_ALPHA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 5;
	 break;
      case GL_LUMINANCE:
      case GL_RGB:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ADD);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 5;
	 break;

      case GL_INTENSITY:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ADD);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ADD);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 6;
	 break;

      case GL_LUMINANCE_ALPHA:
      case GL_RGBA:
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ADD);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_MODULATE);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][4] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       texel_op);
	 imesa->TexBlend[curTex][5] = (STATE3D_MAP_BLEND_ARG_CMD(curTex) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG2 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 6;
	 break;
      default:
	 /* Always set to passthru if something is funny */
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
      }
      break;
   default:
	 /* Always set to passthru if something is funny */
	 imesa->TexBlend[curTex][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_COLOR |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       DISABLE_TEX_CNTRL_STAGE |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				       TEXPIPE_ALPHA |
				       ENABLE_TEXOUTPUT_WRT_SEL |
				       TEXOP_OUTPUT_CURRENT |
				       TEXOP_SCALE_1X |
				       TEXOP_MODIFY_PARMS |
				       TEXBLENDOP_ARG1);
	 imesa->TexBlend[curTex][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_COLOR |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlend[curTex][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				       TEXPIPE_ALPHA |
				       TEXBLEND_ARG1 |
				       TEXBLENDARG_MODIFY_PARMS |
				       TEXBLENDARG_CURRENT);
	 imesa->TexBlendColorPipeNum[curTex] = 0;
	 imesa->TexBlendWordsUsed[curTex] = 4;
	 break;
   }

    if (I830_DEBUG&DEBUG_VERBOSE_TRACE)
       fprintf(stderr, "%s\n", __FUNCTION__);
}

static void i830SetTexEnvCombine(i830ContextPtr imesa,
			     struct gl_texture_unit *texUnit, 
			     GLint unit)
{
   GLuint blendop;
   GLuint ablendop;
   GLuint args_RGB[3];
   GLuint args_A[3];
   GLuint texel_op = GetTexelOp(unit);
   int i;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   switch(texUnit->CombineModeRGB) {
   case GL_REPLACE: blendop = TEXBLENDOP_ARG1; break;
   case GL_MODULATE: blendop = TEXBLENDOP_MODULATE; break;
   case GL_ADD: blendop = TEXBLENDOP_ADD; break;
   case GL_ADD_SIGNED_EXT: blendop = TEXBLENDOP_ADDSIGNED; break;
   case GL_INTERPOLATE_EXT: blendop = TEXBLENDOP_BLEND; break;
   default: return;
   }

   switch(texUnit->CombineScaleShiftRGB) {
   case 0: blendop |= TEXOP_SCALE_1X; break;
   case 1: blendop |= TEXOP_SCALE_2X; break;
   case 2: blendop |= TEXOP_SCALE_4X; break;
   default: return;
   }

   switch(texUnit->CombineModeA) {
   case GL_REPLACE: ablendop = TEXBLENDOP_ARG1; break;
   case GL_MODULATE: ablendop = TEXBLENDOP_MODULATE; break;
   case GL_ADD: ablendop = TEXBLENDOP_ADD; break;
   case GL_ADD_SIGNED_EXT: ablendop = TEXBLENDOP_ADDSIGNED; break;
   case GL_INTERPOLATE_EXT: ablendop = TEXBLENDOP_BLEND; break;
   default: return;
   }

   switch(texUnit->CombineScaleShiftA) {
   case 0: ablendop |= TEXOP_SCALE_1X; break;
   case 1: ablendop |= TEXOP_SCALE_2X; break;
   case 2: ablendop |= TEXOP_SCALE_4X; break;
   default: return;
   }

   /* Handle RGB args */
   for(i = 0; i < 3; i++) {
      switch(texUnit->CombineSourceRGB[i]) {
      case GL_TEXTURE: args_RGB[i] = texel_op; break;
      case GL_CONSTANT_EXT: args_RGB[i] = TEXBLENDARG_FACTOR_N; break;
      case GL_PRIMARY_COLOR_EXT: args_RGB[i] = TEXBLENDARG_DIFFUSE; break;
      case GL_PREVIOUS_EXT: args_RGB[i] = TEXBLENDARG_CURRENT; break;
      default: return;
      }

      switch(texUnit->CombineOperandRGB[i]) {
      case GL_SRC_COLOR: args_RGB[i] |= 0; break;
      case GL_ONE_MINUS_SRC_COLOR: args_RGB[i] |= TEXBLENDARG_INV_ARG; break;
      case GL_SRC_ALPHA: args_RGB[i] |= TEXBLENDARG_REPLICATE_ALPHA; break;
      case GL_ONE_MINUS_SRC_ALPHA: 
		args_RGB[i] |= (TEXBLENDARG_REPLICATE_ALPHA | 
				TEXBLENDARG_INV_ARG); 
		break;
      default: return;
      }
   }

   /* Handle A args */
   for(i = 0; i < 3; i++) {
      switch(texUnit->CombineSourceA[i]) {
      case GL_TEXTURE: args_A[i] = texel_op; break;
      case GL_CONSTANT_EXT: args_A[i] = TEXBLENDARG_FACTOR_N; break;
      case GL_PRIMARY_COLOR_EXT: args_A[i] = TEXBLENDARG_DIFFUSE; break;
      case GL_PREVIOUS_EXT: args_A[i] = TEXBLENDARG_CURRENT; break;
      default: return;
      }

      switch(texUnit->CombineOperandA[i]) {
      case GL_SRC_ALPHA: args_A[i] |= 0; break;
      case GL_ONE_MINUS_SRC_ALPHA: args_A[i] |= TEXBLENDARG_INV_ARG; break;
      default: return;
      }
   }

   /* Native Arg1 == Arg0 in GL_EXT_texture_env_combine spec */
   /* Native Arg2 == Arg1 in GL_EXT_texture_env_combine spec */
   /* Native Arg0 == Arg2 in GL_EXT_texture_env_combine spec */

   /* When we render we need to figure out which is the last really enabled
    * tex unit, and put last stage on it
    */

   imesa->TexBlendColorPipeNum[unit] = 0;

   /* Build color pipeline */

   imesa->TexBlend[unit][0] = (STATE3D_MAP_BLEND_OP_CMD(unit) |
			       TEXPIPE_COLOR |
			       ENABLE_TEXOUTPUT_WRT_SEL |
			       TEXOP_OUTPUT_CURRENT |
			       DISABLE_TEX_CNTRL_STAGE |
			       TEXOP_MODIFY_PARMS |
			       blendop);
   imesa->TexBlend[unit][1] = (STATE3D_MAP_BLEND_ARG_CMD(unit) |
			       TEXPIPE_COLOR |
			       TEXBLEND_ARG1 |
			       TEXBLENDARG_MODIFY_PARMS |
			       args_RGB[0]);
   imesa->TexBlend[unit][2] = (STATE3D_MAP_BLEND_ARG_CMD(unit) |
			       TEXPIPE_COLOR |
			       TEXBLEND_ARG2 |
			       TEXBLENDARG_MODIFY_PARMS |
			       args_RGB[1]);
   imesa->TexBlend[unit][3] = (STATE3D_MAP_BLEND_ARG_CMD(unit) |
			       TEXPIPE_COLOR |
			       TEXBLEND_ARG0 |
			       TEXBLENDARG_MODIFY_PARMS |
			       args_RGB[2]);

   /* Build Alpha pipeline */
   imesa->TexBlend[unit][4] = (STATE3D_MAP_BLEND_OP_CMD(unit) |
			       TEXPIPE_ALPHA |
			       ENABLE_TEXOUTPUT_WRT_SEL |
			       TEXOP_OUTPUT_CURRENT |
			       TEXOP_MODIFY_PARMS |
			       ablendop);
   imesa->TexBlend[unit][5] = (STATE3D_MAP_BLEND_ARG_CMD(unit) |
			       TEXPIPE_ALPHA |
			       TEXBLEND_ARG1 |
			       TEXBLENDARG_MODIFY_PARMS |
			       args_A[0]);
   imesa->TexBlend[unit][6] = (STATE3D_MAP_BLEND_ARG_CMD(unit) |
			       TEXPIPE_ALPHA |
			       TEXBLEND_ARG2 |
			       TEXBLENDARG_MODIFY_PARMS |
			       args_A[1]);
   imesa->TexBlend[unit][7] = (STATE3D_MAP_BLEND_ARG_CMD(unit) |
			       TEXPIPE_ALPHA |
			       TEXBLEND_ARG0 |
			       TEXBLENDARG_MODIFY_PARMS |
			       args_A[2]);

   {
      GLubyte r, g, b, a;
      GLfloat *fc = texUnit->EnvColor;

      FLOAT_COLOR_TO_UBYTE_COLOR(r, fc[RCOMP]);
      FLOAT_COLOR_TO_UBYTE_COLOR(g, fc[GCOMP]);
      FLOAT_COLOR_TO_UBYTE_COLOR(b, fc[BCOMP]);
      FLOAT_COLOR_TO_UBYTE_COLOR(a, fc[ACOMP]);

      imesa->TexBlend[unit][8] = STATE3D_COLOR_FACTOR_CMD(unit);
      imesa->TexBlend[unit][9] =  ((a << 24) |
				   (r << 16) |
				   (g << 8) |
				   b);
   }
   imesa->TexBlendWordsUsed[unit] = 10;
}

static void i830UpdateTexState( GLcontext *ctx, int unit )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   struct gl_texture_object	*tObj;
   i830TextureObjectPtr t;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   tObj = ctx->Texture.Unit[unit].Current;

   if ( tObj != ctx->Texture.Unit[unit].CurrentD[2] ) 
      tObj = 0;

   /* XXX grantham - need to change this shift if "TEXTURE" flags change 
       in mesa/src/types.h */
   if (!(ctx->Texture.ReallyEnabled & (0xf << (4 * unit)))
       || !tObj || !tObj->Complete) {
       return;
   }

   t = tObj->DriverData;
  
   if (!t) {
      t = i830CreateTexObj( imesa, tObj );
      if (!t) return;
   }

   i830TexSetUnit( t, unit );
    
   if (t->dirty_images) {
      if(unit == 0) imesa->dirty |= I830_UPLOAD_TEX0_IMAGE;
      if(unit == 1) imesa->dirty |= I830_UPLOAD_TEX1_IMAGE;
   }

   if((t->Setup[I830_TEXREG_MI1] & ((1<<26)|(1<<25)|(1<<24))) ==
      MAPSURF_8BIT_INDEXED) {
      /* Texture palette needs updated, need to do this in a smarter
       * way, since it will always be loaded each time paletted textures
       * are used, and texture state is reeval'ed.
       */
      if(0) fprintf(stderr, "\n\n\nUpdating texture palette\n");
      if(!ctx->Texture.SharedPalette) {
	 imesa->dirty |= I830_UPLOAD_TEX_PALETTE_N(unit);
	 if(0) fprintf(stderr, "per texobj palette\n");
      } else {
	 imesa->dirty |= I830_UPLOAD_TEX_PALETTE_SHARED;
	 if(0) fprintf(stderr, "shared palette\n");
      }
   }

   imesa->CurrentTexObj[unit] = t;
   t->bound = 1;

   imesa->TexEnabledMask |= I830_TEX_UNIT_ENABLED(unit);
   /* We only want the last texture unit, so we don't or this flag */
   imesa->LastTexEnabled = I830_TEX_UNIT_ENABLED(unit);

   /* Can't do this, we aren't locked here.  Causes bad bad bad things */
#if 0
   if (t->MemBlock)
      i830UpdateTexLRU( imesa, t );
#endif
  
}

static void i830UpdateTexBlend(GLcontext *ctx, int unit )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   struct gl_texture_unit *texUnit;
   struct gl_texture_object *texObj;
   i830TextureObjectPtr t;
   GLuint col;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s : unit %d\n", __FUNCTION__, unit);

   texUnit = &ctx->Texture.Unit[unit];
   texObj = ctx->Texture.Unit[unit].Current;

   if (!(ctx->Texture.ReallyEnabled & (0xf << (4 * unit)))
       || !texObj || !texObj->Complete) {
      return;
   }

   t = (i830TextureObjectPtr) texObj->DriverData;
   if (!t) {
      t = i830CreateTexObj( imesa, texObj );
      if (!t) return;
   }

   imesa->TexBlendWordsUsed[unit] = 0;
   /* Could handle texenv_combine, register_combiners, combine4, etc */
   if(texUnit->EnvMode == GL_COMBINE_EXT) {
      i830SetTexEnvCombine(imesa,
			   texUnit, 
			   unit);
   } else {
      if(t->image[0].internalFormat == GL_COLOR_INDEX) {
	 if(!ctx->Texture.SharedPalette) {
	    i830SetBlend_GL1_2(imesa, unit, texUnit->EnvMode,
			       t->palette_format);
	 } else {
	    i830SetBlend_GL1_2(imesa, unit, texUnit->EnvMode,
			       imesa->palette_format);

	 }
      } else {
	 i830SetBlend_GL1_2(imesa, unit, texUnit->EnvMode,
			    t->image[0].internalFormat);
      }
      /* This only needs emitted when neccessary, fix it later */

      /* add blend color */
      {
	 GLubyte r, g, b, a;
	 GLfloat *fc = texUnit->EnvColor;

	 FLOAT_COLOR_TO_UBYTE_COLOR(r, fc[RCOMP]);
	 FLOAT_COLOR_TO_UBYTE_COLOR(g, fc[GCOMP]);
	 FLOAT_COLOR_TO_UBYTE_COLOR(b, fc[BCOMP]);
	 FLOAT_COLOR_TO_UBYTE_COLOR(a, fc[ACOMP]);

	 col = ((a << 24) |
		(r << 16) |
		(g << 8) |
		b);
      }       

      {
	 int i;

	 i = imesa->TexBlendWordsUsed[unit];
	 imesa->TexBlend[unit][i++] = STATE3D_COLOR_FACTOR_CMD(unit);	  
	 imesa->TexBlend[unit][i++] = col;

	 imesa->TexBlendWordsUsed[unit] = i;
      }
   }
   if(0) fprintf(stderr, "TexBlendWordsUsed : %d\n", imesa->TexBlendWordsUsed[unit]);
}

void i830UpdateTextureState( GLcontext *ctx )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int pipe_num = 0;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if(ctx->Texture.ReallyEnabled & ~(TEXTURE0_2D|(TEXTURE0_2D<<4))) {
      /* Bits are set for a fallback */
      if(0) fprintf(stderr, "Falling back to software for texturing\n");
      imesa->Fallback |= I830_FALLBACK_TEXTURE;
      return;
   }
   imesa->LastTexEnabled = 0;
   imesa->TexEnabledMask = 0;

   if (imesa->CurrentTexObj[0]) imesa->CurrentTexObj[0]->bound = 0;
   if (imesa->CurrentTexObj[1]) imesa->CurrentTexObj[1]->bound = 0;

   imesa->CurrentTexObj[0] = 0;
   imesa->CurrentTexObj[1] = 0;   

   i830UpdateTexState( ctx, 0 );
   i830UpdateTexState( ctx, 1 );

   i830UpdateTexBlend( ctx, 0 );
   i830UpdateTexBlend( ctx, 1 );

   /* Need to decide the units to set to diffuse by looking at
    * the texture units that actually got enabled */
   if(!(imesa->TexEnabledMask & I830_TEX_UNIT_ENABLED(0))) {
      if(0) fprintf(stderr, "Diffuse got turned on\n");
      if(imesa->LastTexEnabled == 0) 
	imesa->LastTexEnabled = I830_TEX_UNIT_ENABLED(0);

      imesa->TexBlend[0][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
			       TEXPIPE_COLOR |
			       ENABLE_TEXOUTPUT_WRT_SEL |
			       TEXOP_OUTPUT_CURRENT |
			       DISABLE_TEX_CNTRL_STAGE |
			       TEXOP_SCALE_1X |
			       TEXOP_MODIFY_PARMS |
			       TEXBLENDOP_ARG1);
      imesa->TexBlend[0][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
			       TEXPIPE_ALPHA |
			       ENABLE_TEXOUTPUT_WRT_SEL |
			       TEXOP_OUTPUT_CURRENT |
			       TEXOP_SCALE_1X |
			       TEXOP_MODIFY_PARMS |
			       TEXBLENDOP_ARG1);
      imesa->TexBlend[0][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
			       TEXPIPE_COLOR |
			       TEXBLEND_ARG1 |
			       TEXBLENDARG_MODIFY_PARMS |
			       TEXBLENDARG_DIFFUSE);
      imesa->TexBlend[0][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
			       TEXPIPE_ALPHA |
			       TEXBLEND_ARG1 |
			       TEXBLENDARG_MODIFY_PARMS |
			       TEXBLENDARG_DIFFUSE);
      imesa->TexBlendColorPipeNum[0] = 0;
      imesa->TexBlendWordsUsed[0] = 4;
      imesa->dirty |= I830_UPLOAD_TEXBLEND_N(0);
   }

   switch(imesa->LastTexEnabled) {
   case I830_TEX_UNIT_ENABLED(0):
      if(0) fprintf(stderr, "Texture unit 0 is the last stage\n");      
      pipe_num = imesa->TexBlendColorPipeNum[0];
      imesa->TexBlend[0][pipe_num] |= TEXOP_LAST_STAGE;
      break;
   case I830_TEX_UNIT_ENABLED(1):
      if(0) fprintf(stderr, "Texture unit 1 is the last stage\n");      
      pipe_num = imesa->TexBlendColorPipeNum[1];
      imesa->TexBlend[1][pipe_num] |= TEXOP_LAST_STAGE;
      break;      
   default: break;
   }

   /* Forces texture cache flush */
   imesa->dirty |= I830_UPLOAD_CTX;
   if(imesa->TexEnabledMask & I830_TEX_UNIT_ENABLED(0)) {
      if(0) fprintf(stderr, "Enabling Texture unit 0\n");
      imesa->dirty |= (I830_UPLOAD_TEX_N(0) | I830_UPLOAD_TEXBLEND_N(0));
   }
   if(imesa->TexEnabledMask & I830_TEX_UNIT_ENABLED(1)) {
      if(0) fprintf(stderr, "Enabling Texture unit 1\n");
      imesa->dirty |= (I830_UPLOAD_TEX_N(1) | I830_UPLOAD_TEXBLEND_N(1));
   }
}



/*****************************************
 * DRIVER functions
 *****************************************/

/* TEXTURE_ENV_COLOR should be optimized, so we only flush when the
 * color really changes
 */

static void i830TexEnv( GLcontext *ctx, GLenum target, 
			GLenum pname, const GLfloat *param )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   /* Always flush texture state or texture_env_combine doesn't work */
   FLUSH_BATCH(imesa);
   imesa->new_state |= I830_NEW_TEXTURE;
}

static void i830TexImage( GLcontext *ctx, 
			  GLenum target,
			  struct gl_texture_object *tObj, 
			  GLint level,
			  GLint internalFormat,
			  const struct gl_texture_image *image )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   i830TextureObjectPtr t;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if (target != GL_TEXTURE_2D)
      return;

   if (level >= I830_TEX_MAXLEVELS)
      return;

   t = (i830TextureObjectPtr) tObj->DriverData;
   if (t) {
      if (t->bound) FLUSH_BATCH(imesa);
      /* if this is the current object, it will force an update */
      i830DestroyTexObj( imesa, t );
      tObj->DriverData = 0;
      imesa->new_state |= I830_NEW_TEXTURE;
   }
}

static void i830TexSubImage( GLcontext *ctx, GLenum target,
			     struct gl_texture_object *tObj, GLint level,
			     GLint xoffset, GLint yoffset,
			     GLsizei width, GLsizei height,
			     GLint internalFormat,
			     const struct gl_texture_image *image ) 
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   i830TextureObjectPtr t;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if ( target != GL_TEXTURE_2D ) 
      return;
   
   t = (i830TextureObjectPtr) tObj->DriverData;
   if (t) {
      if (t->bound) FLUSH_BATCH( imesa );
      i830DestroyTexObj( imesa, t );
      tObj->DriverData = 0;
      imesa->new_state |= I830_NEW_TEXTURE;
   }
}

static void i830TexParameter( GLcontext *ctx, GLenum target,
			      struct gl_texture_object *tObj,
			      GLenum pname, const GLfloat *params )
{
   i830TextureObjectPtr t = (i830TextureObjectPtr) tObj->DriverData;
   i830ContextPtr imesa = I830_CONTEXT( ctx );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if (!t || target != GL_TEXTURE_2D)
      return;

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
      if (t->bound) FLUSH_BATCH( imesa );
      i830SetTexFilter(imesa, t,tObj->MinFilter,tObj->MagFilter);
      break;

   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
      if (t->bound) FLUSH_BATCH( imesa );
      i830SetTexWrapping(t,tObj->WrapS,tObj->WrapT);
      break;
  
   case GL_TEXTURE_BORDER_COLOR:
      if (t->bound) FLUSH_BATCH( imesa );
      i830SetTexBorderColor(t,tObj->BorderColor);
      break;

   default:
      return;
   }

   imesa->new_state |= I830_NEW_TEXTURE;
}

static void i830BindTexture( GLcontext *ctx, GLenum target,
			     struct gl_texture_object *tObj )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);
   
   FLUSH_BATCH(imesa);

   if (imesa->CurrentTexObj[ctx->Texture.CurrentUnit]) {
      imesa->CurrentTexObj[ctx->Texture.CurrentUnit]->bound = 0;
      imesa->CurrentTexObj[ctx->Texture.CurrentUnit] = 0;  
   }

   imesa->new_state |= I830_NEW_TEXTURE;
}

static void i830DeleteTexture( GLcontext *ctx, struct gl_texture_object *tObj )
{
   i830TextureObjectPtr t = (i830TextureObjectPtr)tObj->DriverData;
   i830ContextPtr imesa = I830_CONTEXT( ctx );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if (t) {

      if (t->bound) {
	 FLUSH_BATCH(imesa);
	 imesa->CurrentTexObj[t->bound-1] = 0;
	 imesa->new_state |= I830_NEW_TEXTURE;
      }

      i830DestroyTexObj(imesa,t);
      tObj->DriverData=0;
   }
}


static GLboolean i830IsTextureResident( GLcontext *ctx, 
					struct gl_texture_object *t )
{
   i830TextureObjectPtr mt;

/*     LOCK_HARDWARE; */
   mt = (i830TextureObjectPtr)t->DriverData;
/*     UNLOCK_HARDWARE; */

   return mt && mt->MemBlock;
}

static void
i830DDTexturePalette(GLcontext *ctx, struct gl_texture_object *tObj)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   i830TextureObjectPtr t;

   imesa->new_state |= I830_NEW_TEXTURE;

   if(tObj) {
      t = tObj->DriverData;

      if(!t) {
	 /* Will be handled elsewhere */
	 return;
      }

      i830ConvertPalette(t->palette, &tObj->Palette);
      t->palette_format = tObj->Palette.Format;
   } else {
      i830ConvertPalette(imesa->palette, &ctx->Texture.Palette);
      imesa->palette_format = ctx->Texture.Palette.Format;
   }
}

void i830DDInitTextureFuncs( GLcontext *ctx )
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   ctx->Driver.TexEnv = i830TexEnv;
   ctx->Driver.TexImage = i830TexImage;
   ctx->Driver.TexSubImage = i830TexSubImage;
   ctx->Driver.BindTexture = i830BindTexture;
   ctx->Driver.DeleteTexture = i830DeleteTexture;
   ctx->Driver.TexParameter = i830TexParameter;
   ctx->Driver.UpdateTexturePalette = i830DDTexturePalette;
   ctx->Driver.IsTextureResident = i830IsTextureResident;
}
