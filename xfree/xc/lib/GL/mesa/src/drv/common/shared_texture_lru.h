#ifndef DRI_TEX_HEAP_H
#define DRI_TEX_HEAP_H


/* Private struct.
 */
typedef struct {
	unsigned char next, prev; /* indices to form a circular LRU  */
	unsigned char in_use;	  /* owned by a client, or free? */
	int age;		  /* tracked by clients to update local LRU's */
} driTexRegion;


/* This is the global part of the shared texture mechanism.
 *
 * Do not use this struct directly - declare an equivalent one with a 
 * larger list[] array, tuned to suit your application.
 *
 * Your expanded struct should be placed in the driver-specific
 * portion of the sarea.
 */
struct {
	int globalAge;
	driTexRegion list[1];	/* drivers will want to define a larger list */
} driGlobalList; 


/* This is the client-private part of the mechanism.
 *
 * Clients will place one or more of these structs in their driver
 * context struct to manage one or more global texture heaps.  All
 * fields except print_local_lru must be filled in.
 */
struct dri_tex_heap_t {

	int heapId;		/* client-supplied identifier */
	void *driverContext;	/* pointer to the client's context private */
	int size;		/* heap size in bytes */
	int logGranularity;	/* log base 2 of size of single heap region */
	int nrRegions;	        /* number of elements in global list */
	driGlobalList *shared;	/* pointer to sarea driGlobalList struct */
	int localAge;	        /* initialize to zero */
	
	/* Callback to the client to let it know a region of texture
	 * space has changed age.  The client must integrate this
	 * information with its local texture knowledge, in particular
	 * checking whether any of its own textures have been
	 * invalidated.
	 */
	void (*textures_gone)( void *driverContext,
			       int heapId,
			       int offset, 
			       int size,
			       int inUse );

	/* Optional hook for debugging.
	 */
	void (*print_local_lru)( void *driverContext,
				 int heapId );
} driTexHeap;


#define DRI_AGE_TEXTURES( heap )			\
   if ((heap)->localAge > (heap)->shared->globalAge)	\
      driAgeTextures( heap );


/* This should be called whenever there has been contention on the
 * hardware lock.  Clients can shortcircuit this slightly by using
 * DRI_AGE_TEXTURES, above.
 */
void driAgeTextures( driTexHeap *heap );

#endif

				
