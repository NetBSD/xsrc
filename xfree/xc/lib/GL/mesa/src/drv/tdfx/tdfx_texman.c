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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_texman.c,v 1.4 2001/08/18 02:51:07 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include "tdfx_context.h"
#include "tdfx_tex.h"
#include "tdfx_texman.h"

#define BAD_ADDRESS	((FxU32) -1)

/* Verify the consistancy of the texture memory manager.
 * This involves:
 *    Traversing all texture objects and computing total memory used.
 *    Traverse the free block list and computing total memory free.
 *    Compare the total free and total used amounts to the total memory size.
 *    Make various assertions about the results.
 */
static void tdfxTMVerifyFreeList( tdfxContextPtr fxMesa, FxU32 unit )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   struct gl_texture_object *texObj;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;
   tdfxMemRange *block;
   int prevStart = -1, prevEnd = -1;
   int totalFree = 0;
   int numObj = 0, numRes = 0;
   int totalUsed = 0;

   for ( block = tss->freeRanges[unit] ; block ; block = block->next ) {
      assert( block->endAddr > 0 );
      assert( block->startAddr <= tss->totalTexMem[unit] );
      assert( block->endAddr <= tss->totalTexMem[unit] );
      assert( (int) block->startAddr > prevStart );
      assert( (int) block->startAddr >= prevEnd );
      prevStart = (int) block->startAddr;
      prevEnd = (int) block->endAddr;
      totalFree += (block->endAddr - block->startAddr);
   }
   assert( totalFree == tss->freeTexMem[unit] );

   for ( texObj = ss->TexObjectList ; texObj ; texObj = texObj->Next ) {
      tdfxTexObjPtr t = TDFX_TEXTURE_DATA(texObj);
      numObj++;
      if ( t ) {
	 if ( t->isInTM ) {
	    numRes++;
	    assert( t->range[0] );
	    if ( t->range[unit] )
	       totalUsed += (t->range[unit]->endAddr - t->range[unit]->startAddr);
	 } else {
	    assert(!t->range[0]);
	 }
      }
   }

   fprintf( stderr,
	    "totalFree: %d  totalUsed: %d  totalMem: %d #objs=%d  #res=%d\n",
	    tss->freeTexMem[unit], totalUsed, tss->totalTexMem[unit],
	    numObj, numRes );

   assert( totalUsed + totalFree == tss->totalTexMem[unit] );
}

static void tdfxTMDumpTexMem( tdfxContextPtr fxMesa )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;
   struct gl_texture_object *texObj;
   tdfxMemRange *r;
   FxU32 prev;

   printf( "DUMP Objects:\n" );
   for ( texObj = ss->TexObjectList ; texObj ; texObj = texObj->Next ) {
      tdfxTexObjPtr t = TDFX_TEXTURE_DATA(texObj);

      if ( t && t->isInTM ) {
	 printf( "Obj %8p: %4d  info = %p\n", texObj, texObj->Name, t );

	 printf( "  isInTM=%d  whichTMU=%ld  lastTimeUsed=%d\n",
		 t->isInTM, t->whichTMU, t->lastTimeUsed );
	 printf( "    tm[0] = %p", t->range[0] );
	 assert( t->range[0] );
	 if ( t->range[0] ) {
	    printf( "  tm startAddr = %ld  endAddr = %ld",
		    t->range[0]->startAddr,
		    t->range[0]->endAddr );
	 }
	 printf( "\n" );
	 printf( "    tm[1] = %p", t->range[1] );
	 if ( t->range[1] ) {
	    printf( "  tm startAddr = %ld  endAddr = %ld",
		    t->range[1]->startAddr,
		    t->range[1]->endAddr );
	 }
	 printf( "\n" );
      }
   }

   tdfxTMVerifyFreeList( fxMesa, 0 );
   tdfxTMVerifyFreeList( fxMesa, 1 );

   printf( "Free memory unit 0:  %d bytes\n", tss->freeTexMem[0] );
   prev = 0;
   for ( r = tss->freeRanges[0] ; r ; r = r->next ) {
      printf( "%8p:  start %8ld  end %8ld  size %8ld  gap %8ld\n",
	      r, r->startAddr, r->endAddr, r->endAddr - r->startAddr,
	      r->startAddr - prev );
      prev = r->endAddr;
   }

   printf( "Free memory unit 1:  %d bytes\n", tss->freeTexMem[1] );
   prev = 0;
   for ( r = tss->freeRanges[1] ; r ; r = r->next ) {
      printf( "%8p:  start %8ld  end %8ld  size %8ld  gap %8ld\n",
	      r, r->startAddr, r->endAddr, r->endAddr - r->startAddr,
	      r->startAddr - prev );
      prev = r->endAddr;
   }
}


#ifdef TEXSANITY
static void fubar( void )
{
   /* GH: What am I meant to do??? */
}

/* Sanity Check
 */
static void sanity( tdfxContextPtr fxMesa )
{
   tdfxMemRange *tmp, *prev, *pos;

   prev = 0;
   tmp = fxMesa->freeRanges[0];
   while ( tmp ) {
      if ( !tmp->startAddr && !tmp->endAddr ) {
	 fprintf( stderr, "Textures fubar\n" );
	 fubar();
      }
      if ( tmp->startAddr >= tmp->endAddr ) {
	 fprintf( stderr, "Node fubar\n" );
	 fubar();
      }
      if ( prev && ( prev->startAddr >= tmp->startAddr ||
		     prev->endAddr > tmp->startAddr ) ) {
	 fprintf( stderr, "Sorting fubar\n" );
	 fubar();
      }
      prev = tmp;
      tmp = tmp->next;
   }

   prev = 0;
   tmp = fxMesa->freeRanges[1];
   while ( tmp ) {
      if ( !tmp->startAddr && !tmp->endAddr ) {
	 fprintf( stderr, "Textures fubar\n" );
	 fubar();
      }
      if ( tmp->startAddr >= tmp->endAddr ) {
	 fprintf( stderr, "Node fubar\n" );
	 fubar();
      }
      if ( prev && ( prev->startAddr >= tmp->startAddr ||
		     prev->endAddr > tmp->startAddr ) ) {
	 fprintf( stderr, "Sorting fubar\n" );
	 fubar();
      }
      prev = tmp;
      tmp = tmp->next;
   }
}
#endif


/* Allocate and initialize a new MemRange struct.  Try to allocate it
 * from the pool of free MemRange nodes rather than malloc.
 */
static tdfxMemRange *
tdfxTMNewRangeNode( tdfxContextPtr fxMesa, FxU32 start, FxU32 end )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;
   tdfxMemRange *range;

   _glthread_LOCK_MUTEX( ss->Mutex );
   if ( tss && tss->rangePool ) {
      range = tss->rangePool;
      tss->rangePool = tss->rangePool->next;
   } else {
      range = MALLOC( sizeof(tdfxMemRange) );
   }
   _glthread_UNLOCK_MUTEX( ss->Mutex );

   if ( !range ) {
      if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
	 fprintf( stderr, __FUNCTION__ ": out of memory!\n" );
      return NULL;
   }

   range->startAddr = start;
   range->endAddr = end;
   range->next = NULL;

   return range;
}


/* Initialize texture memory.  We take care of one or both TMU's here.
 */
void tdfxTMInit( tdfxContextPtr fxMesa )
{
   GLcontext *ctx = fxMesa->glCtx;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
      fprintf( stderr, __FUNCTION__ "\n" );

   if ( !ctx->Shared->DriverData ) {
      const char *extensions;
      tdfxSharedStatePtr tss = CALLOC_STRUCT( tdfx_shared_state );

      if ( !tss )
	 return;

      LOCK_HARDWARE( fxMesa );

      extensions = fxMesa->Glide.grGetString( GR_EXTENSION );

      if ( strstr( extensions, " TEXUMA " ) ) {
	 FxU32 start, end;

	 tss->umaTexMemory = GL_TRUE;

	 fxMesa->Glide.grEnable( GR_TEXTURE_UMA_EXT );

	 start = fxMesa->Glide.grTexMinAddress( 0 );
	 end = fxMesa->Glide.grTexMaxAddress( 0 );

	 if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
	    fprintf( stderr, "   UMA tex memory: %d\n", (int)(end - start) );

	 tss->totalTexMem[0] = end - start;
	 tss->totalTexMem[1] = 0;
	 tss->freeTexMem[0] = end - start;
	 tss->freeTexMem[1] = 0;
	 tss->freeRanges[0] = tdfxTMNewRangeNode( fxMesa, start, end );
	 tss->freeRanges[1] = NULL;
      } else {
	 int unit;

	 tss->umaTexMemory = GL_FALSE;

	 for ( unit = 0 ; unit < fxMesa->numTMUs ; unit++ ) {
	    FxU32 start, end;

	    start = fxMesa->Glide.grTexMinAddress( unit );
	    end = fxMesa->Glide.grTexMaxAddress( unit );

	    tss->totalTexMem[unit] = end - start;
	    tss->freeTexMem[unit] = end - start;
	    tss->freeRanges[unit] = tdfxTMNewRangeNode( fxMesa, start, end );

	    if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
	       fprintf( stderr, "   Split tex memory: %d\n",
			(int)(end - start) );
	 }
      }

      UNLOCK_HARDWARE( fxMesa );

      tss->rangePool = NULL;
      ctx->Shared->DriverData = tss;

      if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
	 fprintf( stderr, "  init UMA: %d\n", tss->umaTexMemory );
   }
}


/* Clean-up texture memory before destroying context.
 */
void tdfxTMClose( tdfxContextPtr fxMesa )
{
   GLcontext *ctx = fxMesa->glCtx;

   if ( ctx->Shared->RefCount == 1 && fxMesa->driDrawable ) {
      /* RefCount will soon go to zero, free our 3dfx stuff */
      tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ctx->Shared->DriverData;
      int unit;
      tdfxMemRange *tmp, *next;

      /* Deallocate the pool of free tdfxMemRange nodes */
      tmp = tss->rangePool;
      while ( tmp ) {
	 next = tmp->next;
	 FREE( tmp );
	 tmp = next;
      }

      /* Delete the texture memory block tdfxMemRange nodes */
      for ( unit = 0 ; unit < fxMesa->numTMUs ; unit++ ) {
	 tmp = tss->freeRanges[unit];
	 while ( tmp ) {
	    next = tmp->next;
	    FREE( tmp );
	    tmp = next;
	 }
      }

      FREE( tss );
      ctx->Shared->DriverData = NULL;
   }
}



/* Delete a tdfxMemRange struct.
 * We keep a linked list of free/available tdfxMemRange structs to
 * avoid extra malloc/free calls.
 */
#define DELETE_RANGE_NODE( tss, range )					\
do {									\
   (range)->next = (tss)->rangePool;					\
   (tss)->rangePool = (range);						\
} while (0)

/* When we've run out of texture memory we have to throw out an
 * existing texture to make room for the new one.  This function
 * determins the texture to throw out.
 */
static struct gl_texture_object *
tdfxTMFindOldestObject( tdfxContextPtr fxMesa, FxU32 unit )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   const GLuint bindNumber = fxMesa->texBindNumber;
   struct gl_texture_object *oldestObj, *texObj, *lowestPriorityObj;
   GLfloat lowestPriority;
   GLuint oldestAge;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
      fprintf( stderr, __FUNCTION__ "\n" );

   oldestObj = NULL;
   oldestAge = 0;

   lowestPriority = 1.0F;
   lowestPriorityObj = NULL;

   for ( texObj = ss->TexObjectList ; texObj ; texObj = texObj->Next ) {
      tdfxTexObjPtr t = TDFX_TEXTURE_DATA(texObj);

      if ( t && t->isInTM &&
	   ( ( t->whichTMU == unit ) ||
	     ( t->whichTMU == TDFX_TMU_BOTH ) ||
	     ( t->whichTMU == TDFX_TMU_SPLIT ) ) ) {
	 GLuint age, lastTime;

	 assert( t->range[0] );
	 lastTime = t->lastTimeUsed;

	 if ( lastTime > bindNumber ) {
	    /* TODO: check wrap around */
	    age = bindNumber + (UINT_MAX - lastTime + 1);
	 } else {
	    age = bindNumber - lastTime;
	 }
	 if ( age >= oldestAge ) {
	    oldestAge = age;
	    oldestObj = texObj;
	 }

	 /* examine priority */
	 if ( texObj->Priority < lowestPriority ) {
	    lowestPriority = texObj->Priority;
	    lowestPriorityObj = texObj;
	 }
      }
   }

   if ( lowestPriority < 1.0 ) {
      ASSERT( lowestPriorityObj );
      if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
	 fprintf( stderr, "discard %d pri=%f\n",
		  lowestPriorityObj->Name, lowestPriority );
      return lowestPriorityObj;
   } else {
      if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE )
	 fprintf( stderr, "discard %d age=%d\n",
		  oldestObj->Name, oldestAge );
      return oldestObj;
   }
}


/* Find the address (offset?) at which we can store a new texture.
 * <unit> is the texture unit.
 * <size> is the texture size in bytes.
 */
static FxU32 tdfxTMFindStartAddr( tdfxContextPtr fxMesa,
				  FxU32 unit, FxU32 size )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;
   struct gl_texture_object *texObj;
   tdfxMemRange *prev, *block;
   FxU32 result;

   if ( tss->umaTexMemory ) {
      assert( unit == TDFX_TMU0 );
   }

   _glthread_LOCK_MUTEX( ss->Mutex );
   while ( 1 ) {
      prev = NULL;
      block = tss->freeRanges[unit];

      while ( block ) {
	 if ( block->endAddr - block->startAddr >= size ) {
	    /* The texture will fit here */
	    result = block->startAddr;
	    block->startAddr += size;
	    if ( block->startAddr == block->endAddr ) {
	       /* Remove this node since it's empty */
	       if ( prev ) {
		  prev->next = block->next;
	       } else {
		  tss->freeRanges[unit] = block->next;
	       }
	       DELETE_RANGE_NODE( tss, block );
	    }
	    tss->freeTexMem[unit] -= size;
	    _glthread_UNLOCK_MUTEX( ss->Mutex );
	    return result;
	 }
	 prev = block;
	 block = block->next;
      }

      /* We failed to find a block large enough to accomodate <size> bytes.
       * Find the oldest texObject and free it.
       */
      texObj = tdfxTMFindOldestObject( fxMesa, unit );
      if ( texObj ) {
	 tdfxTMMoveOutTMLocked( fxMesa, texObj );
	 fxMesa->stats.texSwaps++;
      } else {
	 gl_problem( NULL, "tdfx driver: extreme texmem fragmentation" );
	 _glthread_UNLOCK_MUTEX( ss->Mutex );
	 return BAD_ADDRESS;
      }
   }

   /* never get here, but play it safe */
   _glthread_UNLOCK_MUTEX( ss->Mutex );
   return BAD_ADDRESS;
}


/* Remove the given tdfxMemRange node from hardware texture memory.
 */
static void tdfxTMRemoveRangeLocked( tdfxContextPtr fxMesa,
				     FxU32 unit, tdfxMemRange *range )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;
   tdfxMemRange *block, *prev;

   if ( tss->umaTexMemory ) {
      assert( unit == TDFX_TMU0 );
   }

   if ( !range )
      return;

   if ( range->startAddr == range->endAddr ) {
      DELETE_RANGE_NODE( tss, range );
      return;
   }
   tss->freeTexMem[unit] += range->endAddr - range->startAddr;

   /* find position in linked list to insert this tdfxMemRange node */
   prev = NULL;
   block = tss->freeRanges[unit];
   while ( block ) {
      assert( range->startAddr != block->startAddr );
      if ( range->startAddr > block->startAddr ) {
	 prev = block;
	 block = block->next;
      } else {
	 break;
      }
   }

   /* Insert the free block, combine with adjacent blocks when possible */
   range->next = block;
   if ( block ) {
      if ( range->endAddr == block->startAddr ) {
	 /* Combine */
	 block->startAddr = range->startAddr;
	 DELETE_RANGE_NODE( tss, range );
	 range = block;
      }
   }
   if ( prev ) {
      if ( prev->endAddr == range->startAddr ) {
	 /* Combine */
	 prev->endAddr = range->endAddr;
	 prev->next = range->next;
	 DELETE_RANGE_NODE( tss, range );
      } else {
	 prev->next = range;
      }
   } else {
      tss->freeRanges[unit] = range;
   }
}


/* Allocate space for a texture image.
 * <tmu> is the texture unit
 * <texmemsize> is the number of bytes to allocate
 */
static tdfxMemRange *
tdfxTMAllocTexMem( tdfxContextPtr fxMesa, FxU32 unit, FxU32 size )
{
   tdfxMemRange *range = NULL;
   FxU32 start;

   start = tdfxTMFindStartAddr( fxMesa, unit, size );

   if ( start != BAD_ADDRESS ) {
      range = tdfxTMNewRangeNode( fxMesa, start, start + size );
   } else {
      fprintf( stderr,
	       "tdfxTMAllocTexMem returned NULL!  unit=%ld size=%ld\n",
	       unit, size );
   }
   return range;
}


/* Download (copy) the given texture data (all mipmap levels) into the
 * Voodoo's texture memory.  The texture memory must have already been
 * allocated.
 */
void tdfxTMDownloadTextureLocked( tdfxContextPtr fxMesa,
				  struct gl_texture_object *tObj )
{
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);
   FxU32 targetTMU;
   GLint l;

   assert( tObj );
   assert( t );

   targetTMU = t->whichTMU;

   switch ( targetTMU ) {
   case TDFX_TMU0:
   case TDFX_TMU1:
      if ( t->range[targetTMU] ) {
	 for ( l = t->minLevel ; l <= t->maxLevel && t->image[l].data ; l++ ) {
	    GrLOD_t glideLod = t->info.largeLodLog2 - l + tObj->BaseLevel;

	    fxMesa->Glide.grTexDownloadMipMapLevel( targetTMU,
				      t->range[targetTMU]->startAddr,
				      glideLod,
				      t->info.largeLodLog2,
				      t->info.aspectRatioLog2,
				      t->info.format,
				      GR_MIPMAPLEVELMASK_BOTH,
				      t->image[l].data );
	 }
      }
      break;

   case TDFX_TMU_SPLIT:
      if ( t->range[TDFX_TMU0] && t->range[TDFX_TMU1] ) {
	 for ( l = t->minLevel ; l <= t->maxLevel && t->image[l].data ; l++ ) {
	    GrLOD_t glideLod = t->info.largeLodLog2 - l + tObj->BaseLevel;

	    fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU0,
				      t->range[TDFX_TMU0]->startAddr,
				      glideLod,
				      t->info.largeLodLog2,
				      t->info.aspectRatioLog2,
				      t->info.format,
				      GR_MIPMAPLEVELMASK_ODD,
				      t->image[l].data );

	    fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU1,
				      t->range[TDFX_TMU1]->startAddr,
				      glideLod,
				      t->info.largeLodLog2,
				      t->info.aspectRatioLog2,
				      t->info.format,
				      GR_MIPMAPLEVELMASK_EVEN,
				      t->image[l].data );
	 }
      }
      break;

   case TDFX_TMU_BOTH:
      if ( t->range[TDFX_TMU0] && t->range[TDFX_TMU1] ) {
	 for ( l = t->minLevel ; l <= t->maxLevel && t->image[l].data ; l++ ) {
	    GrLOD_t glideLod = t->info.largeLodLog2 - l + tObj->BaseLevel;

	    fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU0,
				      t->range[TDFX_TMU0]->startAddr,
				      glideLod,
				      t->info.largeLodLog2,
				      t->info.aspectRatioLog2,
				      t->info.format,
				      GR_MIPMAPLEVELMASK_BOTH,
				      t->image[l].data );

	    fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU1,
				      t->range[TDFX_TMU1]->startAddr,
				      glideLod,
				      t->info.largeLodLog2,
				      t->info.aspectRatioLog2,
				      t->info.format,
				      GR_MIPMAPLEVELMASK_BOTH,
				      t->image[l].data );
	 }
      }
      break;

   default:
      gl_problem( NULL, "error in tdfxTMDownloadTexture: bad unit" );
      return;
   }
}


void tdfxTMReloadMipMapLevelLocked( GLcontext *ctx,
				    struct gl_texture_object *tObj,
				    GLint level )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);
   GrLOD_t glideLod;
   FxU32 unit;

   ASSERT( t->isInTM );

   unit = t->whichTMU;
   glideLod =  t->info.largeLodLog2 - level + tObj->BaseLevel;

   switch ( unit ) {
   case TDFX_TMU0:
   case TDFX_TMU1:
      fxMesa->Glide.grTexDownloadMipMapLevel( unit,
				t->range[unit]->startAddr,
				glideLod,
				t->info.largeLodLog2,
				t->info.aspectRatioLog2,
				t->info.format,
				GR_MIPMAPLEVELMASK_BOTH,
				t->image[level].data );
      break;

   case TDFX_TMU_SPLIT:
      fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU0,
				t->range[GR_TMU0]->startAddr,
				glideLod,
				t->info.largeLodLog2,
				t->info.aspectRatioLog2,
				t->info.format,
				GR_MIPMAPLEVELMASK_ODD,
				t->image[level].data );

      fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU1,
				t->range[GR_TMU1]->startAddr,
				glideLod,
				t->info.largeLodLog2,
				t->info.aspectRatioLog2,
				t->info.format,
				GR_MIPMAPLEVELMASK_EVEN,
				t->image[level].data );
      break;

   case TDFX_TMU_BOTH:
      fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU0,
				t->range[GR_TMU0]->startAddr,
				glideLod,
				t->info.largeLodLog2,
				t->info.aspectRatioLog2,
				t->info.format,
				GR_MIPMAPLEVELMASK_BOTH,
				t->image[level].data );

      fxMesa->Glide.grTexDownloadMipMapLevel( GR_TMU1,
				t->range[GR_TMU1]->startAddr,
				glideLod,
				t->info.largeLodLog2,
				t->info.aspectRatioLog2,
				t->info.format,
				GR_MIPMAPLEVELMASK_BOTH,
				t->image[level].data );
      break;

   default:
      gl_problem( ctx, "error in tdfxTMReloadMipMapLevel(): wrong unit" );
      break;
   }
}


/* Allocate space for the given texture in texture memory then
 * download (copy) it into that space.
 */
void tdfxTMMoveInTMLocked( tdfxContextPtr fxMesa,
			    struct gl_texture_object *tObj, FxU32 targetTMU )
{
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);
   FxU32 size;

   fxMesa->stats.reqTexUpload++;

   if ( t->isInTM ) {
      if ( t->whichTMU == targetTMU )
	 return;

      if ( targetTMU == TDFX_TMU_SPLIT || t->whichTMU == TDFX_TMU_SPLIT ) {
	 tdfxTMMoveOutTMLocked( fxMesa, tObj );
      } else {
	 if ( t->whichTMU == TDFX_TMU_BOTH )
	    return;
	 targetTMU = TDFX_TMU_BOTH;
      }
   }

   t->whichTMU = targetTMU;

   switch ( targetTMU ) {
   case TDFX_TMU0:
   case TDFX_TMU1:
      size = fxMesa->Glide.grTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH, &t->info );
      t->range[targetTMU] = tdfxTMAllocTexMem(fxMesa, targetTMU, size);
      break;

   case TDFX_TMU_SPLIT:
      size = fxMesa->Glide.grTexTextureMemRequired( GR_MIPMAPLEVELMASK_ODD, &t->info );
      t->range[TDFX_TMU0] = tdfxTMAllocTexMem( fxMesa, TDFX_TMU0, size );
      if ( t->range[TDFX_TMU0] )
	 fxMesa->stats.memTexUpload += size;

      size = fxMesa->Glide.grTexTextureMemRequired( GR_MIPMAPLEVELMASK_EVEN, &t->info );
      t->range[TDFX_TMU1] = tdfxTMAllocTexMem( fxMesa, TDFX_TMU1, size );
      break;

   case TDFX_TMU_BOTH:
      size = fxMesa->Glide.grTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH, &t->info );
      t->range[TDFX_TMU0] = tdfxTMAllocTexMem( fxMesa, TDFX_TMU0, size );
      if ( t->range[TDFX_TMU0] )
	 fxMesa->stats.memTexUpload += size;

      size = fxMesa->Glide.grTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH, &t->info );
      t->range[TDFX_TMU1] = tdfxTMAllocTexMem( fxMesa, TDFX_TMU1, size );
      break;

   default:
      gl_problem( NULL, "error in tdfxTMMoveInTM() -> bad unit (%d)" );
      return;
   }

   t->reloadImages = GL_TRUE;
   t->isInTM = GL_TRUE;

   fxMesa->stats.texUpload++;
}


/* Move the given texture out of hardware texture memory.
 * This deallocates the texture's memory space.
 */
void tdfxTMMoveOutTMLocked( tdfxContextPtr fxMesa,
			    struct gl_texture_object *tObj )
{
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);

   if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE ) {
      fprintf( stderr, __FUNCTION__ "( %p (%d) )\n", tObj, tObj->Name );
      tdfxTMVerifyFreeList( fxMesa, 0 );
      tdfxTMVerifyFreeList( fxMesa, 1 );
   }

   if ( !t || !t->isInTM )
      return;

   switch ( t->whichTMU ) {
   case TDFX_TMU0:
   case TDFX_TMU1:
      tdfxTMRemoveRangeLocked( fxMesa, t->whichTMU, t->range[t->whichTMU] );
      break;

   case TDFX_TMU_SPLIT:
   case TDFX_TMU_BOTH:
      assert( !tss->umaTexMemory );
      tdfxTMRemoveRangeLocked( fxMesa, TDFX_TMU0, t->range[TDFX_TMU0] );
      tdfxTMRemoveRangeLocked( fxMesa, TDFX_TMU1, t->range[TDFX_TMU1] );
      break;

   default:
      gl_problem( NULL, "tdfx driver: bad unit in tdfxTMMOveOutTM()" );
      return;
   }

   t->isInTM = GL_FALSE;
   t->range[0] = NULL;
   t->range[1] = NULL;
   t->whichTMU = TDFX_TMU_NONE;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE ) {
      tdfxTMVerifyFreeList( fxMesa, 0 );
      tdfxTMVerifyFreeList( fxMesa, 1 );
   }
}


void tdfxTMFreeTextureLocked( tdfxContextPtr fxMesa,
			      struct gl_texture_object *tObj )
{
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);

   if ( t ) {
      int i;
      tdfxTMMoveOutTMLocked( fxMesa, tObj );
      for ( i = 0 ; i < MAX_TEXTURE_LEVELS ; i++ ) {
	 if ( t->image[i].original.data ) FREE( t->image[i].original.data );
	 if ( t->image[i].rescaled.data ) FREE( t->image[i].rescaled.data );
      }
      FREE( t );
      tObj->DriverData = NULL;
   }

   if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE ) {
      tdfxTMVerifyFreeList( fxMesa, 0 );
      tdfxTMVerifyFreeList( fxMesa, 1 );
   }
}


/* After a context switch this function will be called to restore
 * texture memory for the new context.
 */
void tdfxTMRestoreTexturesLocked( tdfxContextPtr fxMesa )
{
   GLcontext *ctx = fxMesa->glCtx;
   struct gl_texture_object *tObj;
   int i;

   for ( tObj = ctx->Shared->TexObjectList ; tObj ; tObj = tObj->Next ) {
      tdfxTexObjPtr t = TDFX_TEXTURE_DATA( tObj );
      if ( t && t->isInTM ) {
	 for ( i = 0 ; i < MAX_TEXTURE_UNITS ; i++ ) {
	    if ( ctx->Texture.Unit[i].Current == tObj ) {
	       tdfxTMDownloadTextureLocked( fxMesa, tObj );
	       break;
	    }
	 }
	 if ( i == MAX_TEXTURE_UNITS ) {
	    tdfxTMMoveOutTMLocked( fxMesa, tObj );
	 }
      }
   }

   if ( TDFX_DEBUG & DEBUG_VERBOSE_TEXTURE ) {
      tdfxTMVerifyFreeList( fxMesa, 0 );
      tdfxTMVerifyFreeList( fxMesa, 1 );
   }
}
