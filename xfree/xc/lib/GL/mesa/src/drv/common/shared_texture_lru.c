#include <stdio.h>
#include "shared_texture_lru.h"

/* (Re)initialize the global circular LRU list.  The last element
 * in the array (heap->nrRegions) is the sentinal.  Keeping it
 * at the end of the array allows the other elements of the array
 * to be addressed rationally when looking up objects at a
 * particular location in texture memory.  
 */
static void resetGlobalLRU( driHeapPtr heap )
{
   driTexRegion *list = heap->shared->list;
   int sz = 1 << heap->logGranularity;
   int i;

   heap->localAge = ++heap->shared->texAge;

   for (i = 0 ; (i+1) * sz <= heap->size ; i++) {
      list[i].prev = i-1;
      list[i].next = i+1;
      list[i].age = heap->shared->texAge;   
   }

   i--;
   list[0].prev = heap->nrRegions;
   list[i].prev = i-1;
   list[i].next = heap->nrRegions;
   list[heap->nrRegions].prev = i;
   list[heap->nrRegions].next = 0;
}

/* Called by the client whenever it touches a local texture.  
 */
void driUpdateHeap( driHeapPtr heap, int start, int end )
{
   driTexRegion *list = heap->shared->list;
   int i;

   heap->localAge = ++heap->shared->globalAge;

   for (i = start ; i <= end ; i++) 
   {
      list[i].in_use = 1;
      list[i].age = heap->localAge;

      /* remove_from_list(i)
       */
      list[(unsigned)list[i].next].prev = list[i].prev;
      list[(unsigned)list[i].prev].next = list[i].next;
      
      /* insert_at_head(list, i)
       */
      list[i].prev = heap->nrRegions;
      list[i].next = list[heap->nrRegions].next;
      list[(unsigned)list[heap->nrRegions].next].prev = i;
      list[heap->nrRegions].next = i;
   }
}


/* Called by the client on lock contention to determine whether
 * textures have been stolen
 */
void driAgeTextures( driHeapPtr heap )
{
   driTexRegion *list = heap->shared->list;
   int sz = 1 << (heap->logGranularity);
   int i, nr = 0;

   /* Have to go right round from the back to ensure stuff ends up
    * LRU in the local list...  Fix with a cursor pointer.
    */
   for (i = list[heap->nrRegions].prev ; 
	i != heap->nrRegions && nr < heap->nrRegions ; 
	i = list[i].prev, nr++)
   {
      if (list[i].age > heap->localAge) 
	 heap->texturesGone( heap->driverContext, heap->heapId, i * sz, sz, 1); 
   }

   /* Loop or uninitialized heap detected.  Reset.
    */
   if (nr == heap->nrRegions) {
      heap->texturesGone( heap->driverContext, heap->heapId, 0, heap->size, 0);
      resetGlobalLRU( heap );
   }
}

