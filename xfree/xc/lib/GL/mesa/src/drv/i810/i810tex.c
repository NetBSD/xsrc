/*
 * GLX Hardware Device Driver for Intel i810
 * Copyright (C) 1999 Keith Whitwell
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
 * KEITH WHITWELL, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810tex.c,v 1.7 2001/10/31 22:50:24 tsi Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>

#include "mm.h"
#include "i810context.h"
#include "i810tex.h"
#include "i810log.h"
#include "i810ioctl.h"
#include "simple_list.h"
#include "enums.h"

static void i810SetTexWrapping(i810TextureObjectPtr tex, GLenum s, GLenum t)
{
   unsigned int val = tex->Setup[I810_TEXREG_MCS];

   val &= ~(MCS_U_STATE_MASK|MCS_V_STATE_MASK);
   val |= (MCS_U_WRAP|MCS_V_WRAP);
   
   if (s != GL_REPEAT) 
      val ^= (MCS_U_WRAP^MCS_U_CLAMP);

   if (t != GL_REPEAT) 
      val ^= (MCS_V_WRAP^MCS_V_CLAMP);

   tex->Setup[I810_TEXREG_MCS] = val;
}

static void i810SetTexFilter(i810ContextPtr imesa, 
			     i810TextureObjectPtr t, 
			     GLenum minf, GLenum magf)
{
   GLuint LastLevel;

   switch (minf) {
   case GL_NEAREST:
      I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
		     MF_MIN_MASK | MF_MIP_MASK,
		     MF_MIN_NEAREST | MF_MIP_NONE);
      break;
   case GL_LINEAR:
      I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
		     MF_MIN_MASK | MF_MIP_MASK,
		     MF_MIN_LINEAR | MF_MIP_NONE);
      break;
   case GL_NEAREST_MIPMAP_NEAREST:
      I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
		     MF_MIN_MASK | MF_MIP_MASK,
		     MF_MIN_NEAREST | MF_MIP_NEAREST);
      break;
   case GL_LINEAR_MIPMAP_NEAREST:
      I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
		     MF_MIN_MASK | MF_MIP_MASK,
		     MF_MIN_LINEAR | MF_MIP_NEAREST);
      break;
   case GL_NEAREST_MIPMAP_LINEAR:
      /* This is quite a performance hit - may want to make this
       * choice user-configurable, otherwise the 815 may look slower
       * than the 810 (despite having much better image quality).
       */
      if (IS_I815(imesa)) {
	 I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
			MF_MIN_MASK | MF_MIP_MASK,
			MF_MIN_NEAREST | MF_MIP_LINEAR );
      } else {
	 I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
			MF_MIN_MASK | MF_MIP_MASK,
			MF_MIN_NEAREST | MF_MIP_DITHER );
      }
      break;
   case GL_LINEAR_MIPMAP_LINEAR:
      if (IS_I815(imesa)) {
	 I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
			MF_MIN_MASK | MF_MIP_MASK,
			MF_MIN_LINEAR  | MF_MIP_LINEAR );
      } else {
	 I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
			MF_MIN_MASK | MF_MIP_MASK,
			MF_MIN_LINEAR  | MF_MIP_DITHER );
      }
      break;
   default:
      i810Error("i810SetTexFilter(): not supported min. filter %d\n",(int)minf);
      break;
   }

   switch (magf) {
   case GL_NEAREST:
      I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
		     MF_MAG_MASK,MF_MAG_NEAREST);
      break;
   case GL_LINEAR:
      I810_SET_FIELD(t->Setup[I810_TEXREG_MF],
		     MF_MAG_MASK,MF_MAG_LINEAR);
      break;
   default:
      i810Error("i810SetTexFilter(): not supported mag. filter %d\n",(int)magf);
      break;
   }  

   if (t->globj->MinFilter != GL_NEAREST && 
       t->globj->MinFilter != GL_LINEAR) {
      LastLevel = t->max_level;
   } else {
      LastLevel = t->min_level;
   }

   I810_SET_FIELD(t->Setup[I810_TEXREG_MLL],
		  MLL_MAX_MIP_MASK,
		  (t->min_level << (MLL_MAX_MIP_SHIFT+4)));

   I810_SET_FIELD(t->Setup[I810_TEXREG_MLL],
		  MLL_MIN_MIP_MASK,
		  (LastLevel << MLL_MIN_MIP_SHIFT));

   /* See OpenGL 1.2 specification */
   if (magf == GL_LINEAR && (minf == GL_NEAREST_MIPMAP_NEAREST || 
			     minf == GL_NEAREST_MIPMAP_LINEAR))
   {
      /* c = 0.5 */
      I810_SET_FIELD(t->Setup[I810_TEXREG_MLC],
		     MLC_LOD_BIAS_MASK,
		     0x10);
   } else {
      /* c = 0 */
      I810_SET_FIELD(t->Setup[I810_TEXREG_MLC],
		     MLC_LOD_BIAS_MASK,
		     0x0);
   }
}


/* Need a fallback ?
 */
static void i810SetTexBorderColor(i810TextureObjectPtr t, GLubyte color[4])
{
/*    t->Setup[I810_TEXREG_TEXBORDERCOL] =  */
/*      I810PACKCOLOR8888(color[0],color[1],color[2],color[3]); */
}



static void ReplicateMesaTexState(i810ContextPtr imesa,
				  i810TextureObjectPtr t,
                                  struct gl_texture_object *mesatex)
{
   i810SetTexWrapping(t,mesatex->WrapS,mesatex->WrapT);
   i810SetTexFilter(imesa, t,mesatex->MinFilter,mesatex->MagFilter);
   i810SetTexBorderColor(t,mesatex->BorderColor);
}

static i810TextureObjectPtr i810CreateTexObj(i810ContextPtr imesa,
					     struct gl_texture_object *tObj)
{
   i810TextureObjectPtr t;
   GLuint height, width, pitch, i, textureFormat, log_pitch;
   struct gl_texture_image *image;

   image = tObj->Image[ 0 ];
   if ( !image ) {
      fprintf(stderr, "no image at level zero - not creating texobj\n");
      return 0;
   }
   
   t = (i810TextureObjectPtr) calloc(1,sizeof(*t));
   if (!t)
      return 0;

   switch( image->Format ) {
   case GL_RGB:
   case GL_LUMINANCE:
      t->texelBytes = 2;
      textureFormat = MI1_FMT_16BPP | MI1_PF_16BPP_RGB565;
      break;
   case GL_ALPHA:
   case GL_LUMINANCE_ALPHA:
   case GL_INTENSITY:
   case GL_RGBA:
      t->texelBytes = 2;
      textureFormat = MI1_FMT_16BPP | MI1_PF_16BPP_ARGB4444;
      break;
   case GL_COLOR_INDEX:
      textureFormat = MI1_FMT_8CI | MI1_PF_8CI_ARGB4444;
      t->texelBytes = 1;
      break;
   default:
      i810Error( "i810CreateTexObj: bad image->Format\n" );
      free( t );      
      return 0;	
   }


   /* Figure out the size now (and count the levels).  Upload won't be done
    * until later.
    */ 
   width = image->Width * t->texelBytes;
   for (pitch = 32, log_pitch=2 ; pitch < width ; pitch *= 2 )
      log_pitch++;
   
   t->dirty_images = 0;
   t->bound = 0;
   
   for ( height = i = 0 ; i < I810_TEX_MAXLEVELS && tObj->Image[i] ; i++ ) {
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

   t->Setup[I810_TEXREG_MI0] = GFX_OP_MAP_INFO;

   t->Setup[I810_TEXREG_MI1] = (MI1_MAP_0 |
				textureFormat |
				log_pitch);			 

   t->Setup[I810_TEXREG_MI2] = (MI2_DIMENSIONS_ARE_LOG2 |
				(image->HeightLog2 << 16) |
				(image->WidthLog2));

   t->Setup[I810_TEXREG_MI3] = 0;
  
   t->Setup[I810_TEXREG_MLC] = (GFX_OP_MAP_LOD_CTL | 
				MLC_MAP_0 |
				MLC_DITHER_WEIGHT_FULL |
				MLC_UPDATE_LOD_BIAS |
				0x0);

   t->Setup[I810_TEXREG_MLL] = (GFX_OP_MAP_LOD_LIMITS |
				MLL_MAP_0  |
				MLL_UPDATE_MAX_MIP | 
				(t->min_level << MLL_MAX_MIP_SHIFT) |
				MLL_UPDATE_MIN_MIP |
				t->max_level);

   /* I think this is context state, really.
    */
   t->Setup[I810_TEXREG_MCS] = (GFX_OP_MAP_COORD_SETS |
				MCS_COORD_0 |
				MCS_UPDATE_NORMALIZED |
				MCS_NORMALIZED_COORDS |
				MCS_UPDATE_V_STATE |
				MCS_V_WRAP |
				MCS_UPDATE_U_STATE |
				MCS_U_WRAP);

   t->Setup[I810_TEXREG_MF] = (GFX_OP_MAP_FILTER |
			       MF_MAP_0 |
			       MF_UPDATE_ANISOTROPIC |
			       0 |
			       MF_UPDATE_MIP_FILTER |
			       MF_MIP_NEAREST |
			       MF_UPDATE_MAG_FILTER |
			       MF_MAG_NEAREST |
			       MF_UPDATE_MIN_FILTER |
			       MF_MIN_NEAREST);

   t->current_unit = 0;

   ReplicateMesaTexState(imesa, t,tObj);
   tObj->DriverData = t;
   imesa->dirty |= I810_UPLOAD_CTX;
   make_empty_list( t );
   return t;
}

void i810DestroyTexObj(i810ContextPtr imesa, i810TextureObjectPtr t)
{
   if (!t) return;

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


static void i810SwapOutTexObj(i810ContextPtr imesa, i810TextureObjectPtr t)
{
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
static void i810UploadTexLevel( i810TextureObjectPtr t, int level )
{
   const struct gl_texture_image *image = t->image[level].image;
   int i,j;

   if (I810_DEBUG & DEBUG_VERBOSE_LRU)
      fprintf(stderr, "i810UploadTexLevel %d, BufAddr %p offset %x\n",
	      level, t->BufAddr, t->image[level].offset);

   /* Need triangle (rather than pixel) fallbacks to simulate this using
    * normal textured triangles.
    *
    * DO THIS IN DRIVER STATE MANAGMENT, not hardware state.
    *
   if (image->Border != 0) 
      i810Error("Not supported texture border %d.\n", (int) image->Border);
    */

   switch (t->image[level].internalFormat) {
   case GL_RGB:
   {
      GLushort *dst = (GLushort *)(t->BufAddr + t->image[level].offset);
      GLubyte  *src = (GLubyte *)image->Data;

      for (j = 0 ; j < image->Height ; j++, dst += (t->Pitch/2)) {
	 for (i = 0 ; i < image->Width ; i++) {
	    dst[i] = I810PACKCOLOR565(src[0],src[1],src[2]);
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
	    dst[i] = I810PACKCOLOR4444(src[0],src[1],src[2],src[3]);
	    src += 4;
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
	    dst[i] = I810PACKCOLOR565(src[0],src[0],src[0]);
	    src ++;
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
	    dst[i] = I810PACKCOLOR4444(src[0],src[0],src[0],src[0]);
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
	    dst[i] = I810PACKCOLOR4444(src[0],src[0],src[0],src[1]);
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
	    dst[i] = I810PACKCOLOR4444(255,255,255,src[0]);
	    src += 1;
	 }
      }
   }
   break;

   /* TODO: Translate color indices *now*:
    */
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
      i810Error("Not supported texture format %s\n",
		gl_lookup_enum_by_nr(image->Format));
   }
}



void i810PrintLocalLRU( i810ContextPtr imesa ) 
{
   i810TextureObjectPtr t;
   int sz = 1 << (imesa->i810Screen->logTextureGranularity);

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

void i810PrintGlobalLRU( i810ContextPtr imesa )
{
   int i, j;
   drm_i810_tex_region_t *list = imesa->sarea->texList;

   for (i = 0, j = I810_NR_TEX_REGIONS ; i < I810_NR_TEX_REGIONS ; i++) {
      fprintf(stderr, "list[%d] age %d next %d prev %d\n",
	      j, list[j].age, list[j].next, list[j].prev);
      j = list[j].next;
      if (j == I810_NR_TEX_REGIONS) break;
   }
   
   if (j != I810_NR_TEX_REGIONS)
      fprintf(stderr, "Loop detected in global LRU\n");
}


void i810ResetGlobalLRU( i810ContextPtr imesa )
{
   drm_i810_tex_region_t *list = imesa->sarea->texList;
   int sz = 1 << imesa->i810Screen->logTextureGranularity;
   int i;

   /* (Re)initialize the global circular LRU list.  The last element
    * in the array (I810_NR_TEX_REGIONS) is the sentinal.  Keeping it
    * at the end of the array allows it to be addressed rationally
    * when looking up objects at a particular location in texture
    * memory.  
    */
   for (i = 0 ; (i+1) * sz <= imesa->i810Screen->textureSize ; i++) {
      list[i].prev = i-1;
      list[i].next = i+1;
      list[i].age = 0;
   }

   i--;
   list[0].prev = I810_NR_TEX_REGIONS;
   list[i].prev = i-1;
   list[i].next = I810_NR_TEX_REGIONS;
   list[I810_NR_TEX_REGIONS].prev = i;
   list[I810_NR_TEX_REGIONS].next = 0;
   imesa->sarea->texAge = 0;
}


static void i810UpdateTexLRU( i810ContextPtr imesa, i810TextureObjectPtr t ) 
{
   int i;
   int logsz = imesa->i810Screen->logTextureGranularity;
   int start = t->MemBlock->ofs >> logsz;
   int end = (t->MemBlock->ofs + t->MemBlock->size - 1) >> logsz;
   drm_i810_tex_region_t *list = imesa->sarea->texList;
   
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
      list[i].prev = I810_NR_TEX_REGIONS;
      list[i].next = list[I810_NR_TEX_REGIONS].next;
      list[(unsigned)list[I810_NR_TEX_REGIONS].next].prev = i;
      list[I810_NR_TEX_REGIONS].next = i;
   }
}


/* Called for every shared texture region which has increased in age
 * since we last held the lock.
 *
 * Figures out which of our textures have been ejected by other clients,
 * and pushes a placeholder texture onto the LRU list to represent 
 * the other client's textures.  
 */
void i810TexturesGone( i810ContextPtr imesa,
		       GLuint offset, 
		       GLuint size,
		       GLuint in_use ) 
{
   i810TextureObjectPtr t, tmp;
   
   foreach_s ( t, tmp, &imesa->TexObjList ) {

      if (t->MemBlock->ofs >= offset + size ||
	  t->MemBlock->ofs + t->MemBlock->size <= offset)
	 continue;

      /* It overlaps - kick it off.  Need to hold onto the currently bound
       * objects, however.
       */
      if (t->bound)
	 i810SwapOutTexObj( imesa, t );
      else
	 i810DestroyTexObj( imesa, t );
   }

   
   if (in_use) {
      t = (i810TextureObjectPtr) calloc(1,sizeof(*t));
      if (!t) return;

      t->MemBlock = mmAllocMem( imesa->texHeap, size, 0, offset);      
      insert_at_head( &imesa->TexObjList, t );
   }
}





/* This is called with the lock held.  May have to eject our own and/or
 * other client's texture objects to make room for the upload.
 */
int i810UploadTexImages( i810ContextPtr imesa, i810TextureObjectPtr t )
{
   int i;
   int ofs;

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
	    i810PrintLocalLRU( imesa );
	    return -1;
	 }

	 if (imesa->TexObjList.prev == &(imesa->TexObjList)) {
 	    fprintf(stderr, "Failed to upload texture, sz %d\n", t->totalSize);
	    mmDumpMemInfo( imesa->texHeap );
	    return -1;
	 }
	 
	 i810DestroyTexObj( imesa, imesa->TexObjList.prev );
      }
 
      ofs = t->MemBlock->ofs;
      t->Setup[I810_TEXREG_MI3] = imesa->i810Screen->textureOffset + ofs;
      t->BufAddr = imesa->i810Screen->tex.map + ofs;
      imesa->dirty |= I810_UPLOAD_CTX;
   }

   /* Let the world know we've used this memory recently.
    */
   i810UpdateTexLRU( imesa, t );

   if (I810_DEBUG & DEBUG_VERBOSE_LRU)
      fprintf(stderr, "dispatch age: %d age freed memory: %d\n",
	      GET_DISPATCH_AGE(imesa), imesa->dirtyAge);

   if (imesa->dirtyAge >= GET_DISPATCH_AGE(imesa)) 
      i810WaitAgeLocked( imesa, imesa->dirtyAge );
   

   if (t->dirty_images) {
      if (I810_DEBUG & DEBUG_VERBOSE_LRU)
	 fprintf(stderr, "*");

      for (i = t->min_level ; i <= t->max_level ; i++)
	 if (t->dirty_images & (1<<i)) 
	    i810UploadTexLevel( t, i );
   }


   t->dirty_images = 0;
   return 0;
}

static void i810TexSetUnit( i810TextureObjectPtr t, GLuint unit )
{
   if (t->current_unit == unit) return;

   t->Setup[I810_TEXREG_MI1] ^= (MI1_MAP_0 ^ MI1_MAP_1);
   t->Setup[I810_TEXREG_MLC] ^= (MLC_MAP_0 ^ MLC_MAP_1);
   t->Setup[I810_TEXREG_MLL] ^= (MLL_MAP_0 ^ MLL_MAP_1);
   t->Setup[I810_TEXREG_MCS] ^= (MCS_COORD_0 ^ MCS_COORD_1);
   t->Setup[I810_TEXREG_MF]  ^= (MF_MAP_0 ^ MF_MAP_1);

   t->current_unit = unit;
}




static void i810UpdateTex0State( GLcontext *ctx )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   struct gl_texture_object	*tObj;
   i810TextureObjectPtr t;
   int ma_modulate_op;
   int format;

   /* disable */
   imesa->Setup[I810_CTXREG_MT] &= ~MT_TEXEL0_ENABLE;
   imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
                                     MC_STAGE_0 |
                                     MC_UPDATE_DEST |
                                     MC_DEST_CURRENT |
                                     MC_UPDATE_ARG1 |
                                     MC_ARG1_ITERATED_COLOR | 
                                     MC_UPDATE_ARG2 |
                                     MC_ARG2_ONE |
                                     MC_UPDATE_OP |
                                     MC_OP_ARG1 );
   imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
                                     MA_STAGE_0 |
                                     MA_UPDATE_ARG1 |
                                     MA_ARG1_ITERATED_ALPHA |
                                     MA_UPDATE_ARG2 |
                                     MA_ARG2_TEX0_ALPHA |
                                     MA_UPDATE_OP |
                                     MA_OP_ARG1 );

   if (ctx->Texture.Unit[0].ReallyEnabled == 0) {
      return;
   }

   tObj = ctx->Texture.Unit[0].Current;
   if (ctx->Texture.Unit[0].ReallyEnabled != TEXTURE0_2D ||
       tObj->Image[tObj->BaseLevel]->Border > 0) {
      /* 1D or 3D texturing enabled, or texture border - fallback */
      imesa->Fallback |= I810_FALLBACK_TEXTURE;
      return;
   }

   /* Do 2D texture setup */

   imesa->Setup[I810_CTXREG_MT] |= MT_TEXEL0_ENABLE;

   t = tObj->DriverData;
   if (!t) {
      t = i810CreateTexObj( imesa, tObj );
      if (!t)
         return;
   }

   if (t->current_unit != 0)
      i810TexSetUnit( t, 0 );
    
   if (t->dirty_images) 
      imesa->dirty |= I810_UPLOAD_TEX0IMAGE;
   
   imesa->CurrentTexObj[0] = t;
   t->bound = 1;

   if (t->MemBlock)
      i810UpdateTexLRU( imesa, t );
  
   format = t->image[0].internalFormat;

   switch (ctx->Texture.Unit[0].EnvMode) {
   case GL_REPLACE:
      if (format == GL_ALPHA) 
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_TEX0_COLOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_ITERATED_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_ARG2 );
      else 
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_TEX0_COLOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_ONE |
					   MC_UPDATE_OP |
					   MC_OP_ARG1 );

      if (format == GL_RGB) {
	 ma_modulate_op = MA_OP_ARG1;
      } else {
	 ma_modulate_op = MA_OP_ARG2;
      }

      imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_0 |
					MA_UPDATE_ARG1 |
					MA_ARG1_ITERATED_ALPHA |
					MA_UPDATE_ARG2 |
					MA_ARG2_TEX0_ALPHA |
					MA_UPDATE_OP |
					ma_modulate_op );
      break;
   case GL_MODULATE:
      imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					MC_STAGE_0 |
					MC_UPDATE_DEST |
					MC_DEST_CURRENT |
					MC_UPDATE_ARG1 |
					MC_ARG1_TEX0_COLOR | 
					MC_UPDATE_ARG2 |
					MC_ARG2_ITERATED_COLOR |
					MC_UPDATE_OP |
					MC_OP_MODULATE );

      if (format == GL_RGB) {
	 ma_modulate_op = MA_OP_ARG1;
      } else {
	 ma_modulate_op = MA_OP_MODULATE;
      }
	
      imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_0 |
					MA_UPDATE_ARG1 |
					MA_ARG1_ITERATED_ALPHA |
					MA_UPDATE_ARG2 |
					MA_ARG2_TEX0_ALPHA |
					MA_UPDATE_OP |
					MA_OP_MODULATE );
      break;

   case GL_ADD:
      if (format == GL_ALPHA) {
         /* Cv = Cf */
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_TEX0_COLOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_ITERATED_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_ARG2 );
      }
      else {
         /* Cv = Cf + Ct */
         imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
                                           MC_STAGE_0 |
                                           MC_UPDATE_DEST |
                                           MC_DEST_CURRENT |
                                           MC_UPDATE_ARG1 |
                                           MC_ARG1_TEX0_COLOR | 
                                           MC_UPDATE_ARG2 |
                                           MC_ARG2_ITERATED_COLOR |
                                           MC_UPDATE_OP |
                                           MC_OP_ADD );
      }

      /* alpha */
      if (format == GL_ALPHA ||
          format == GL_LUMINANCE_ALPHA ||
          format == GL_RGBA) {
         /* Av = Af * At */
         imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
                                           MA_STAGE_0 |
                                           MA_UPDATE_ARG1 |
                                           MA_ARG1_ITERATED_ALPHA |
                                           MA_UPDATE_ARG2 |
                                           MA_ARG2_TEX0_ALPHA |
                                           MA_UPDATE_OP |
                                           MA_OP_MODULATE );
      }
      else if (format == GL_LUMINANCE || format == GL_RGB) {
         /* Av = Af */
         imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
                                           MA_STAGE_0 |
                                           MA_UPDATE_ARG1 |
                                           MA_ARG1_ITERATED_ALPHA |
                                           MA_UPDATE_ARG2 |
                                           MA_ARG2_ITERATED_ALPHA |
                                           MA_UPDATE_OP |
                                           MA_OP_ARG1 );
      }
      else {
         /* Av = Af + At */
         imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
                                           MA_STAGE_0 |
                                           MA_UPDATE_ARG1 |
                                           MA_ARG1_ITERATED_ALPHA |
                                           MA_UPDATE_ARG2 |
                                           MA_ARG2_TEX0_ALPHA |
                                           MA_UPDATE_OP |
                                           MA_OP_ADD );
      }
      break;

   case GL_DECAL:
      if (format == GL_RGB) {
         /* C = Ct */
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_COLOR_FACTOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_TEX0_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_ARG2 );

      } else {
         /* RGBA or undefined result */
         /* C = Cf*(1-At)+Ct*At */
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_TEX0_COLOR |
					   MC_UPDATE_ARG2 |
					   MC_ARG2_ITERATED_COLOR | 
					   MC_UPDATE_OP |
					   MC_OP_LIN_BLEND_TEX0_ALPHA );
      }

      /* Av = Af */
      imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_0 |
					MA_UPDATE_ARG1 |
					MA_ARG1_ITERATED_ALPHA |
					MA_UPDATE_ARG2 |
					MA_ARG2_ITERATED_ALPHA |
					MA_UPDATE_OP |
					MA_OP_ARG1 );
      break;
   case GL_BLEND:
      if (format == GL_ALPHA) 
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_TEX0_COLOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_ITERATED_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_ARG2 );
      else 
	 imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_0 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_COLOR_FACTOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_ITERATED_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_LIN_BLEND_TEX0_COLOR );

      /* alpha */
      if (format == GL_LUMINANCE || format == GL_RGB) {
         /* Av = Af */
         imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
                                           MA_STAGE_0 |
                                           MA_UPDATE_ARG1 |
                                           MA_ARG1_ITERATED_ALPHA |
                                           MA_UPDATE_ARG2 |
                                           MA_ARG2_ITERATED_ALPHA |
                                           MA_UPDATE_OP |
                                           MA_OP_ARG1 );
      }
      else if (format == GL_INTENSITY) {
         /* Av = Af(1-It)+AcIt */
	 imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
					   MA_STAGE_0 |
					   MA_UPDATE_ARG1 |
					   MA_ARG1_ALPHA_FACTOR |
					   MA_UPDATE_ARG2 |
					   MA_ARG2_ITERATED_ALPHA |
					   MA_UPDATE_OP |
					   MA_OP_LIN_BLEND_TEX0_ALPHA );
      } else {
         /* Av = AfAt */
	 imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
					   MA_STAGE_0 |
					   MA_UPDATE_ARG1 |
					   MA_ARG1_TEX0_ALPHA |
					   MA_UPDATE_ARG2 |
					   MA_ARG2_ITERATED_ALPHA |
					   MA_UPDATE_OP |
					   MA_OP_MODULATE );
      }
      break;

   default:
      fprintf(stderr, "unknown tex env mode");
      exit(1);
      break;			
   }
}



static void i810UpdateTex1State( GLcontext *ctx )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   struct gl_texture_object	*tObj;
   i810TextureObjectPtr t;
   int ma_modulate_op, format;

   /* disable */
   imesa->Setup[I810_CTXREG_MT] &= ~MT_TEXEL1_ENABLE;
   imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
                                     MC_STAGE_1 |
                                     MC_UPDATE_DEST |
                                     MC_DEST_CURRENT |
                                     MC_UPDATE_ARG1 |
                                     MC_ARG1_ONE | 
                                     MC_ARG1_DONT_REPLICATE_ALPHA |
                                     MC_ARG1_DONT_INVERT |
                                     MC_UPDATE_ARG2 |
                                     MC_ARG2_ONE |
                                     MC_ARG2_DONT_REPLICATE_ALPHA |
                                     MC_ARG2_DONT_INVERT |
                                     MC_UPDATE_OP |
                                     MC_OP_DISABLE );
   imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
                                     MA_STAGE_1 |
                                     MA_UPDATE_ARG1 |
                                     MA_ARG1_CURRENT_ALPHA |
                                     MA_ARG1_DONT_INVERT |
                                     MA_UPDATE_ARG2 |
                                     MA_ARG2_CURRENT_ALPHA |
                                     MA_ARG2_DONT_INVERT |
                                     MA_UPDATE_OP |
                                     MA_OP_ARG1 );

   if (ctx->Texture.Unit[1].ReallyEnabled == 0) {
      return;
   }

   tObj = ctx->Texture.Unit[1].Current;
   if (ctx->Texture.Unit[1].ReallyEnabled != TEXTURE0_2D ||
       tObj->Image[tObj->BaseLevel]->Border > 0) {
      /* 1D or 3D texturing enabled, or texture border - fallback */
      imesa->Fallback |= I810_FALLBACK_TEXTURE;
      return;
   }

   /* Do 2D texture setup */

   imesa->Setup[I810_CTXREG_MT] |= MT_TEXEL1_ENABLE;

   t = tObj->DriverData;
   if (!t) {
      t = i810CreateTexObj( imesa, tObj );
      if (!t)
         return;
   }
    
   if (t->current_unit != 1)
      i810TexSetUnit( t, 1 );

   if (t->dirty_images) 
      imesa->dirty |= I810_UPLOAD_TEX1IMAGE;

   imesa->CurrentTexObj[1] = t;
   t->bound = 2;

   if (t->MemBlock)
      i810UpdateTexLRU( imesa, t );

   format = t->image[0].internalFormat;

   switch (ctx->Texture.Unit[1].EnvMode) {
   case GL_REPLACE:
      imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
					MC_STAGE_1 |
					MC_UPDATE_DEST |
					MC_DEST_CURRENT |
					MC_UPDATE_ARG1 |
					MC_ARG1_TEX1_COLOR | 
					MC_UPDATE_ARG2 |
					MC_ARG2_ONE |
					MC_UPDATE_OP |
					MC_OP_ARG1 );

      if (format == GL_RGB) {
	 ma_modulate_op = MA_OP_ARG1;
      } else {
	 ma_modulate_op = MA_OP_ARG2;
      }

      imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_1 |
					MA_UPDATE_ARG1 |
					MA_ARG1_CURRENT_ALPHA |
					MA_UPDATE_ARG2 |
					MA_ARG2_TEX1_ALPHA |
					MA_UPDATE_OP |
					ma_modulate_op );
      break;
   case GL_MODULATE:
      imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
					MC_STAGE_1 |
					MC_UPDATE_DEST |
					MC_DEST_CURRENT |
					MC_UPDATE_ARG1 |
					MC_ARG1_TEX1_COLOR | 
					MC_UPDATE_ARG2 |
					MC_ARG2_CURRENT_COLOR |
					MC_UPDATE_OP |
					MC_OP_MODULATE );

      if (format == GL_RGB) {
	 ma_modulate_op = MA_OP_ARG1;
      } else {
	 ma_modulate_op = MA_OP_MODULATE;
      }
	
      imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_1 |
					MA_UPDATE_ARG1 |
					MA_ARG1_CURRENT_ALPHA |
					MA_UPDATE_ARG2 |
					MA_ARG2_TEX1_ALPHA |
					MA_UPDATE_OP |
					ma_modulate_op );
      break;

   case GL_ADD:
      imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
					MC_STAGE_1 |
					MC_UPDATE_DEST |
					MC_DEST_CURRENT |
					MC_UPDATE_ARG1 |
					MC_ARG1_TEX1_COLOR | 
					MC_UPDATE_ARG2 |
					MC_ARG2_CURRENT_COLOR |
					MC_UPDATE_OP |
					MC_OP_ADD );

      if (format == GL_RGB) {
	 ma_modulate_op = MA_OP_ARG1;
      } else {
	 ma_modulate_op = MA_OP_ADD;
      }
	
      imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_1 |
					MA_UPDATE_ARG1 |
					MA_ARG1_CURRENT_ALPHA |
					MA_UPDATE_ARG2 |
					MA_ARG2_TEX1_ALPHA |
					MA_UPDATE_OP |
					ma_modulate_op );
      break;


   case GL_DECAL:
      if (format == GL_RGB) {
	 imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_1 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_COLOR_FACTOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_TEX1_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_ARG2 );

      } else {
	 imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
					   MC_STAGE_1 |
					   MC_UPDATE_DEST |
					   MC_DEST_CURRENT |
					   MC_UPDATE_ARG1 |
					   MC_ARG1_COLOR_FACTOR | 
					   MC_UPDATE_ARG2 |
					   MC_ARG2_TEX1_COLOR |
					   MC_UPDATE_OP |
					   MC_OP_LIN_BLEND_TEX1_ALPHA );
      }

      imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
					MA_STAGE_1 |
					MA_UPDATE_ARG1 |
					MA_ARG1_ALPHA_FACTOR |
					MA_UPDATE_ARG2 |
					MA_ARG2_ALPHA_FACTOR |
					MA_UPDATE_OP |
					MA_OP_ARG1 );
      break;

   case GL_BLEND:
      imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
					MC_STAGE_1 |
					MC_UPDATE_DEST |
					MC_DEST_CURRENT |
					MC_UPDATE_ARG1 |
					MC_ARG1_COLOR_FACTOR | 
					MC_UPDATE_ARG2 |
					MC_ARG2_CURRENT_COLOR |
					MC_UPDATE_OP |
					MC_OP_LIN_BLEND_TEX1_COLOR );

      if (format == GL_RGB) {
	 imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
					   MA_STAGE_1 |
					   MA_UPDATE_ARG1 |
					   MA_ARG1_ALPHA_FACTOR |
					   MA_UPDATE_ARG2 |
					   MA_ARG2_CURRENT_ALPHA |
					   MA_UPDATE_OP |
					   MA_OP_ARG1 );
      } else {
	 imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
					   MA_STAGE_1 |
					   MA_UPDATE_ARG1 |
					   MA_ARG1_ALPHA_FACTOR |
					   MA_UPDATE_ARG2 |
					   MA_ARG2_ITERATED_ALPHA |
					   MA_UPDATE_OP |
					   MA_OP_LIN_BLEND_TEX1_ALPHA );
      }
      break;

   default:
      fprintf(stderr, "unkown tex 1 env mode\n");
      exit(1);
      break;			
   }
}


void i810UpdateTextureState( GLcontext *ctx )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   if (imesa->CurrentTexObj[0]) imesa->CurrentTexObj[0]->bound = 0;
   if (imesa->CurrentTexObj[1]) imesa->CurrentTexObj[1]->bound = 0;
   imesa->CurrentTexObj[0] = 0;
   imesa->CurrentTexObj[1] = 0;   
   imesa->Fallback &= ~I810_FALLBACK_TEXTURE;
   i810UpdateTex0State( ctx );
   i810UpdateTex1State( ctx );
   I810_CONTEXT( ctx )->dirty |= (I810_UPLOAD_CTX |
                                  I810_UPLOAD_TEX0 | 
                                  I810_UPLOAD_TEX1);
}



/*****************************************
 * DRIVER functions
 *****************************************/

static void i810TexEnv( GLcontext *ctx, GLenum target, 
			GLenum pname, const GLfloat *param )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );

   if (pname == GL_TEXTURE_ENV_MODE) {

      FLUSH_BATCH(imesa);	
      imesa->new_state |= I810_NEW_TEXTURE;

   } else if (pname == GL_TEXTURE_ENV_COLOR) {

      struct gl_texture_unit *texUnit = 
	 &ctx->Texture.Unit[ctx->Texture.CurrentUnit];
      const GLfloat *fc = texUnit->EnvColor;
      GLuint r, g, b, a, col;
      FLOAT_COLOR_TO_UBYTE_COLOR(r, fc[0]);
      FLOAT_COLOR_TO_UBYTE_COLOR(g, fc[1]);
      FLOAT_COLOR_TO_UBYTE_COLOR(b, fc[2]);
      FLOAT_COLOR_TO_UBYTE_COLOR(a, fc[3]);

      col = ((a << 24) | 
	     (r << 16) | 
	     (g <<  8) | 
	     (b <<  0));
    
      if (imesa->Setup[I810_CTXREG_CF1] != col) {
	 FLUSH_BATCH(imesa);	
	 imesa->Setup[I810_CTXREG_CF1] = col;      
	 imesa->dirty |= I810_UPLOAD_CTX;
      }
   } 
}

static void i810TexImage( GLcontext *ctx, 
			  GLenum target,
			  struct gl_texture_object *tObj, 
			  GLint level,
			  GLint internalFormat,
			  const struct gl_texture_image *image )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   i810TextureObjectPtr t;

   if (target != GL_TEXTURE_2D)
      return;

   if (level >= I810_TEX_MAXLEVELS)
      return;

   t = (i810TextureObjectPtr) tObj->DriverData;
   if (t) {
      if (t->bound) FLUSH_BATCH(imesa);
      /* if this is the current object, it will force an update */
      i810DestroyTexObj( imesa, t );
      tObj->DriverData = 0;
      imesa->new_state |= I810_NEW_TEXTURE;
   }
}

static void i810TexSubImage( GLcontext *ctx, GLenum target,
			     struct gl_texture_object *tObj, GLint level,
			     GLint xoffset, GLint yoffset,
			     GLsizei width, GLsizei height,
			     GLint internalFormat,
			     const struct gl_texture_image *image ) 
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   i810TextureObjectPtr t;

   if ( target != GL_TEXTURE_2D ) 
      return;
   
   t = (i810TextureObjectPtr) tObj->DriverData;
   if (t) {
      if (t->bound) FLUSH_BATCH( imesa );
      i810DestroyTexObj( imesa, t );
      tObj->DriverData = 0;
      imesa->new_state |= I810_NEW_TEXTURE;
   }
}

static void i810TexParameter( GLcontext *ctx, GLenum target,
			      struct gl_texture_object *tObj,
			      GLenum pname, const GLfloat *params )
{
   i810TextureObjectPtr t = (i810TextureObjectPtr) tObj->DriverData;
   i810ContextPtr imesa = I810_CONTEXT( ctx );

   if (!t || target != GL_TEXTURE_2D)
      return;

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
      if (t->bound) FLUSH_BATCH( imesa );
      i810SetTexFilter(imesa, t,tObj->MinFilter,tObj->MagFilter);
      break;

   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
      if (t->bound) FLUSH_BATCH( imesa );
      i810SetTexWrapping(t,tObj->WrapS,tObj->WrapT);
      break;
  
   case GL_TEXTURE_BORDER_COLOR:
      if (t->bound) FLUSH_BATCH( imesa );
      i810SetTexBorderColor(t,tObj->BorderColor);
      break;

   default:
      return;
   }

   imesa->new_state |= I810_NEW_TEXTURE;
}

static void i810BindTexture( GLcontext *ctx, GLenum target,
			     struct gl_texture_object *tObj )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   
   FLUSH_BATCH(imesa);

   if (imesa->CurrentTexObj[ctx->Texture.CurrentUnit]) {
      imesa->CurrentTexObj[ctx->Texture.CurrentUnit]->bound = 0;
      imesa->CurrentTexObj[ctx->Texture.CurrentUnit] = 0;  
   }

   imesa->new_state |= I810_NEW_TEXTURE;
}

static void i810DeleteTexture( GLcontext *ctx, struct gl_texture_object *tObj )
{
   i810TextureObjectPtr t = (i810TextureObjectPtr)tObj->DriverData;
   i810ContextPtr imesa = I810_CONTEXT( ctx );

   if (t) {

      if (t->bound) {
	 FLUSH_BATCH(imesa);
	 imesa->CurrentTexObj[t->bound-1] = 0;
	 imesa->new_state |= I810_NEW_TEXTURE;
      }

      i810DestroyTexObj(imesa,t);
      tObj->DriverData=0;
   }
}


static GLboolean i810IsTextureResident( GLcontext *ctx, 
					struct gl_texture_object *t )
{
   i810TextureObjectPtr mt;

/*     LOCK_HARDWARE; */
   mt = (i810TextureObjectPtr)t->DriverData;
/*     UNLOCK_HARDWARE; */

   return mt && mt->MemBlock;
}

void i810DDInitTextureFuncs( GLcontext *ctx )
{
   ctx->Driver.TexEnv = i810TexEnv;
   ctx->Driver.TexImage = i810TexImage;
   ctx->Driver.TexSubImage = i810TexSubImage;
   ctx->Driver.BindTexture = i810BindTexture;
   ctx->Driver.DeleteTexture = i810DeleteTexture;
   ctx->Driver.TexParameter = i810TexParameter;
   ctx->Driver.UpdateTexturePalette = 0;
   ctx->Driver.IsTextureResident = i810IsTextureResident;
}
