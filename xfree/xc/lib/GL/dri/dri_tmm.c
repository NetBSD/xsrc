/* dri_tmm.c -- High-level texture memory management
 * Created: Mon May 10 12:58:20 1999 by faith@precisioninsight.com
 * Revised: Tue May 18 00:38:49 1999 by faith@precisioninsight.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
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
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 * $XFree86: xc/lib/GL/dri/dri_tmm.c,v 1.3 2001/08/27 17:40:57 dawes Exp $
 * 
 *
 * DESCRIPTION
 *
 *
 * FUTURE ENHANCEMENTS
 *
 *
 * REFERENCES
 *
 *
 */

#include <unistd.h>
#include <X11/Xlibint.h>
#include "xf86drm.h"

#include "dri_tmm.h"
#define DRI_TMM_ALLOC(s) Xmalloc(s)
#define DRI_TMM_FREE(p)  Xfree(p)


static void          *driTMMAlloc(int size);
static void          driTMMFree(void *p);
static void          driTMMRemoveLRU(driTMMPtr tmm, driTMMAreaPtr area);
static void          driTMMInsertLRU(driTMMPtr tmm, driTMMAreaPtr area);
static driTMMAreaPtr driTMMGetLRU(driTMMPtr tmm);
static void          driTMMDestroyArea(driTMMAreaPtr area);
static driTMMAreaPtr driTMMCreateArea(unsigned long offset,
				      unsigned long size);
static int           driTMMFreeArea(driTMMPtr tmm, driTMMAreaPtr area);
static driTMMAreaPtr driTMMFindFreeArea(driTMMPtr tmm, unsigned long size);
static driTMMAreaPtr driTMMBreakNewArea(driTMMPtr tmm, unsigned long size);
static driTMMAreaPtr driTMMAllocArea(driTMMPtr tmm,
				     unsigned long size,
				     driTMMImagePtr image);
static int           driTMMComputePriority(driTMMPtr tmm, float priority);
#if 0
static void          driTMMDump(driTMMPtr tmm);
#endif

static void *driTMMAlloc(int size)
{
    void *address = DRI_TMM_ALLOC(size);
#if 0
    printf("%p %d\n", address, size);
#endif
    return address;
}

static void driTMMFree(void *p)
{
#if 0
    printf("%p\n",p);
#endif
    if (p) DRI_TMM_FREE(p);
}

static void driTMMRemoveLRU(driTMMPtr tmm, driTMMAreaPtr area)
{
    driTMMPrioPtr prio;

    prio = &tmm->priolist[area->priority];

				/* Disconnect, if connected */
    if (area->prev)         area->prev->next = area->next;
    if (area->next)         area->next->prev = area->prev;
    if (prio->head == area) prio->head       = area->next;
    if (prio->tail == area) prio->tail       = area->prev;

    area->next   = NULL;
    area->prev   = NULL;
    area->locked = 0;
}

static void driTMMInsertLRU(driTMMPtr tmm, driTMMAreaPtr area)
{
    driTMMPrioPtr prio;

    prio = &tmm->priolist[area->priority];

				/* Disconnect, if connected */
    driTMMRemoveLRU(tmm, area);

				/* Reconnect at end of list */
    if (prio->tail)         prio->tail->next = area;
    if (!prio->head)        prio->head = area;
    area->prev       = prio->tail;
    area->next       = NULL;
    prio->tail       = area;
}

static driTMMAreaPtr driTMMGetLRU(driTMMPtr tmm)
{
    int           i;
    driTMMAreaPtr area;
    
    for (i = 0; i < DRI_TMM_PRIORITIES; i++) {
	for (area = tmm->priolist[i].head; area; area = area->next) {
	    if (!area->locked) {
		driTMMRemoveLRU(tmm, area);
		if (area->image) {
		    area->image->area = NULL;
		    area->image       = NULL;
		}
		return area;
	    }
	}
    }
    return NULL;
}

static void driTMMDestroyArea(driTMMAreaPtr area)
{
    driTMMFree(area);
}

static driTMMAreaPtr driTMMCreateArea(unsigned long offset, unsigned long size)
{
    driTMMAreaPtr area;

    area           = driTMMAlloc(sizeof(*area));
    area->offset   = offset;
    area->size     = size;
    area->priority = DRI_TMM_DEFAULT_PRIORITY;
    area->image    = NULL;
    area->locked   = 0;
    area->prev     = NULL;
    area->next     = NULL;
    
    return area;
}

static int driTMMFreeArea(driTMMPtr tmm, driTMMAreaPtr area)
{
    unsigned long prev_offset, next_offset;
    driTMMAreaPtr prev, next;

    driTMMRemoveLRU(tmm, area);
    if (area->image) {
	area->image->area = NULL;
	area->image       = NULL;
    }

    drmSLLookupNeighbors(tmm->freelist, area->offset,
			 &prev_offset, (void **)&prev,
			 &next_offset, (void **)&next);

    if (prev && prev->offset + prev->size == area->offset) {
				/* Merge with previous area */
	prev->size += area->size;
	driTMMDestroyArea(area);
	if (next && prev->offset + prev->size == next->offset) {
				/* Merge with next area */
	    prev->size += next->size;
	    drmSLDelete(tmm->freelist, next->offset);
	    driTMMDestroyArea(next);
	}
    } else if (next && area->offset + area->size == next->offset) {
				/* Merge with next area */
	drmSLDelete(tmm->freelist, next->offset);
	next->offset =  area->offset;
	next->size   += area->size;
	drmSLInsert(tmm->freelist, next->offset, next);
	driTMMDestroyArea(area);
    } else {
				/* Insert into list */
	drmSLInsert(tmm->freelist, area->offset, area);
    }
    return 0;
}

static driTMMAreaPtr driTMMFindFreeArea(driTMMPtr tmm, unsigned long size)
{
    unsigned long offset;
    driTMMAreaPtr area  = NULL;
    driTMMAreaPtr best  = NULL;
    unsigned long extra = 0;

    if (drmSLFirst(tmm->freelist, &offset, (void **)&area)) {
	do {
	    if (area->size == size) {
		best = area;
		break;
	    } else if (area->size > size) {
		if (!best || area->size - size < extra) {
		    best  = area;
		    extra = area->size - size;
		    if (tmm->strategy == DRI_TMM_FIRST_FIT) break;
		}
	    }
	} while (drmSLNext(tmm->freelist, &offset, (void **)&area));
    }
    if (best) {
	if (best->size > size) {
				/* Split */
	    best->size -= size;
	    return driTMMCreateArea(best->offset + best->size, size);
	} else {
	    drmSLDelete(tmm->freelist, best->offset);
	    return best;
	}
    }
    
    return NULL;
}

static driTMMAreaPtr driTMMBreakNewArea(driTMMPtr tmm, unsigned long size)
{
    unsigned long offset;
    
    if (tmm->current + size <= tmm->end) {
	offset       = tmm->current;
	tmm->current += size;
	return driTMMCreateArea(offset, size);
    }
    return NULL;
}

static driTMMAreaPtr driTMMAllocArea(driTMMPtr tmm, unsigned long size,
				     driTMMImagePtr image)
{
    driTMMAreaPtr area = NULL;
    driTMMAreaPtr victim;

    size += (tmm->alignment - 1) & ~(tmm->alignment - 1);

				/* Is there a free area? */
    if (!area) area = driTMMFindFreeArea(tmm, size);

				/* Is there free memory? */
    if (!area) area = driTMMBreakNewArea(tmm, size);

				/* What can we evict? */
    if (!area) {
	while ((victim = driTMMGetLRU(tmm))) {
	    driTMMFreeArea(tmm, victim);
	    if ((area = driTMMFindFreeArea(tmm, size))) break;
	}
    }

    if (area) {
	area->image   = image;
	area->locked  = 0;
	if (image) {
	    area->priority  = image->priority;
	    image->area   = area;
	} else {
	    area->priority  = tmm->priority;
	}
	driTMMInsertLRU(tmm, area);
    }
	

    return area;
}

static int driTMMComputePriority(driTMMPtr tmm, float priority)
{
    int p = priority * DRI_TMM_PRIORITIES;

    if (p < 0)                   p = 0;
    if (p >= DRI_TMM_PRIORITIES) p = DRI_TMM_PRIORITIES - 1;
    return p;
}

#if 0
static void driTMMDump(driTMMPtr tmm)
{
    driTMMAreaPtr area;
    unsigned long offset;
    int           i;
    
    printf("Start   = %10lu\n", tmm->start);
    printf("Current = %10lu\n", tmm->current);
    printf("End     = %10lu\n", tmm->end);
    printf("Size    = %10lu\n", tmm->size);

    printf("Freelist:\n");
    if (drmSLFirst(tmm->freelist, &offset, (void **)&area)) {
	do {
	    printf( "  %10lu %10lu %3d %p %p\n",
		    area->offset,
		    area->size,
		    area->priority,
		    area->prev,
		    area->next);
	} while (drmSLNext(tmm->freelist, &offset, (void **)&area));
    }

    for (i = 0; i < DRI_TMM_PRIORITIES; i++) {
	printf("Priority %2d:\n", i);
	for (area = tmm->priolist[i].head; area; area = area->next) {
	    printf( "  %10lu %10lu %3d %p %p\n",
		    area->offset,
		    area->size,
		    area->priority,
		    area->prev,
		    area->next);
	}
    }
}
#endif

driTMMPtr driTMMCreate(unsigned long      start,
		       unsigned long      size,
		       unsigned long      word_size, /* in bytes */
		       unsigned long      alignment,
		       driTMMLoadImage    load_image,
		       driTMMLoadSubImage load_subimage)
{
    driTMMPtr tmm;
    int       i;

    tmm                = driTMMAlloc(sizeof(*tmm));

    tmm->start         = start;
    tmm->current       = start;
    tmm->end           = tmm->start + size;
    tmm->size          = size;
    tmm->word_size     = word_size;
    tmm->alignment     = alignment ? alignment : 1;
    tmm->load_image    = load_image;
    tmm->load_subimage = load_subimage;


    tmm->freelist  = drmSLCreate();
    tmm->images    = drmHashCreate();
    for (i = 0; i < DRI_TMM_PRIORITIES; i++) {
	tmm->priolist[i].head = NULL;
	tmm->priolist[i].tail = NULL;
    }
    tmm->priority  = DRI_TMM_DEFAULT_PRIORITY;
    tmm->strategy  = DRI_TMM_BEST_FIT;
    tmm->hits      = 0;
    tmm->misses    = 0;
    
    return tmm;
}

void driTMMDestroy(driTMMPtr tmm)
{
    driTMMImagePtr t;
    unsigned long  key;
    
    if (!tmm) return;
    drmSLDestroy(tmm->freelist);

    if (drmHashFirst(tmm->images, &key, (void *)&t)) {
	do {
	    driTMMDeleteImage(tmm, t);
	} while (drmHashNext(tmm->images, &key, (void *)&t));
    }
    drmHashDestroy(tmm->images);
    
    driTMMFree(tmm);
}

/* NOTE: driTMMInsertImage does *NOT* make the texture image resident. */

extern void *driTMMInsertImage(driTMMPtr           tmm,
			       int                 width,
			       int                 height,
			       int                 texel_width,
			       const unsigned char *image,
			       void                *private)
{
    driTMMImagePtr t;

    if (!tmm) return NULL;
    if (!(t = driTMMAlloc(sizeof(*t)))) return NULL;

    t->width       = width;
    t->height      = height;
    t->texel_width = texel_width;
    t->private     = private;
    
    t->length      = width * height * (texel_width/8);
    t->size        = t->length / tmm->word_size;
    t->priority    = tmm->priority;
    t->area        = NULL;

    if (image) {
	if (!(t->buffer = driTMMAlloc(t->length))) {
	    driTMMFree(t);
	    return NULL;
	}
	memcpy(t->buffer, image, t->length);
    } else {
	t->buffer = NULL;
    }
    
    drmHashInsert(tmm->images, (unsigned long)t, t);

    return t;
}

int driTMMSubImage(driTMMPtr           tmm,
		   void                *image,
		   int                 xoffset,
		   int                 yoffset,
		   int                 subimage_width,
		   int                 subimage_height,
		   const unsigned char *subimage)
{
    driTMMImagePtr t = (driTMMImagePtr)image;
    int            y;

    if (t->buffer && subimage) {
				/* FIXME.  This only works for texel_widths
                                   that are a multiple of 8. */
	for (y = yoffset; y < yoffset + subimage_height; y++) {
	    memcpy(t->buffer + y * t->width + xoffset,
		   subimage + (y - yoffset) * subimage_width,
		   subimage_width * (t->texel_width/8));
	}
    }

#if 0
    printf("0x%08lx w%d h%d tw%d %p x%d y%d sw%d sh%d %p\n",
	   t->area->offset, t->width, t->height, t->texel_width, t->buffer,
	   xoffset, yoffset, subimage_width, subimage_height, subimage);
#endif

				/* Load subimage onto hardware, if
                                   resident.  Note that GL semantics
                                   require that the texture be resident
                                   when a subimage operation is
                                   performed. */
    if (tmm->load_subimage && t->area) {
	(tmm->load_subimage)(t->private,
			     t->area->offset,
			     t->width,
			     t->height,
			     t->texel_width,
			     xoffset,
			     yoffset,
			     subimage_width,
			     subimage_height,
			     subimage);
    }
    return 0;
}

int driTMMDeleteImage(driTMMPtr tmm, void *image)
{
    driTMMImagePtr t = (driTMMImagePtr)image;

    if (!tmm)                                         return -1;
    if (!image)                                       return 0;
    if (drmHashDelete(tmm->images, (unsigned long)t)) return -1;
    
    if (t->area)   driTMMFreeArea(tmm, t->area);
    if (t->buffer) driTMMFree(t->buffer);
    driTMMFree(t);
    return 0;
}

int driTMMDeleteImages(driTMMPtr tmm, int count, void **images)
{
    int i;
    int retcode = 0;

    for (i = 0; i < count; i++) {
	if (driTMMDeleteImage(tmm, images[i])) ++retcode;
    }
    return retcode ? -1 : 0;
}

int driTMMIsImageResident(driTMMPtr tmm, void *image)
{
    driTMMImagePtr t = (driTMMImagePtr)image;

    if (!t)      return 0;	/* Error */
    if (t->area) return 1;	/* Resident */
    return 0;			/* Not resident */
}

/* If all the textures are resident, driTMMAreImagesResident returns 1 and
   the contents of residences are not changed.  If any texture is not
   resident, then driTMMAreImagesResident returns 0 and the corresponding
   elements in reisdences are set to 0.  This behavior mimics that of the
   glAreTexturesResident() function, as described in the OpenGL Programming
   Guide, p. 352.  This seems to conflict with the Reference, so maybe it's
   wrong... */

int driTMMAreImagesResident(driTMMPtr tmm, int count, void **images,
			    int *residences)
{
    int i;
    int retcode = 1;

    for (i = 0; i < count; i++) {
	if (!driTMMIsImageResident(tmm, images[i])) {
	    residences[i] = 0;
	    retcode = 0;
	}
    }
    return retcode;
}

void driTMMPrioritizeImage(driTMMPtr tmm, void *image, float priority)
{
    driTMMImagePtr t = (driTMMImagePtr)image;
    int            p = driTMMComputePriority(tmm, priority);

    if (p != t->priority && t->area) {
	driTMMRemoveLRU(tmm, t->area);
	t->area->priority = t->priority = p;
	driTMMInsertLRU(tmm, t->area);
    }
    t->priority = p;
}
    

void driTMMPrioritizeImages(driTMMPtr tmm, int count, void **images,
			    float *priorities)
{
    int i;
    
    for (i = 0; i < count; i++)
	driTMMPrioritizeImage(tmm, images[i], priorities[i]);
}

void driTMMSetDefaultPriority(driTMMPtr tmm, float priority)
{
    tmm->priority = driTMMComputePriority(tmm, priority);
}

int driTMMMakeImageResident(driTMMPtr tmm, void *image, 
			    unsigned long *address)
{
    driTMMImagePtr t = (driTMMImagePtr)image;
    driTMMAreaPtr  area;

    if (address) *address = 0;

    if (!tmm) return -1; /* Error */
    if (!image) return 0;	/* NULL image is always loaded */
    if (t->area) {
	++tmm->hits;
	if (address) *address = t->area->offset;
	return 0;	/* Already resident */
    }
    ++tmm->misses;
    
    if (!(area = driTMMAllocArea(tmm, t->size, image))) return -1; /* Error*/

    if (address) *address = area->offset;

#if 0
    printf("0x%08lx w%d h%d tw%d %p h%lu m%lu\n",
	   area->offset, t->width, t->height, t->texel_width, t->buffer,
	   tmm->hits, tmm->misses);
#endif

    if (tmm->load_image)
	tmm->load_image(t->private, area->offset, t->width, t->height, 
			t->texel_width, t->buffer);
    return 0;
}

int driTMMMakeImagesResident(driTMMPtr tmm, int count, void **images,
			     unsigned long *addresses)
{
    driTMMImagePtr t;
    int            i;
    int            retcode = 0;

    if (!tmm || !images) return -1;

				/* Lock resident images */
    for (i = 0; i < count; i++) {
	t = (driTMMImagePtr)images[i];
	if (t && t->area) t->area->locked = 1;
    }

				/* Make non-resident images resident */
    for (i = 0; !retcode && i < count; i++) {
	t = (driTMMImagePtr)images[i];
	retcode = driTMMMakeImageResident(tmm, t, &addresses[i]);
	if (t && t->area) t->area->locked = 1;
    }

    if (retcode) {		/* At least one image won't fit, so
				   clear a contiguous segment and try
				   again. */
	unsigned long total = 0;
	
	for (i = 0; i < count; i++) {
	    t = (driTMMImagePtr)images[i];
	    if (t && t->area) driTMMFreeArea(tmm, t->area);
	}
				/* Find out how much room all the textures
                                   will take. */
	for (i = 0; i < count; i++) {
	    t = (driTMMImagePtr)images[i];
	    if (t) {
		total += ((t->size + tmm->alignment - 1)
			  & ~(tmm->alignment - 1));
	    }
	}
				/* Free enough room for that chunk */
	driTMMFreeArea(tmm, driTMMAllocArea(tmm, total, NULL));

				/* Make images resident */
	for (retcode = 0, i = 0; !retcode && i < count; i++) {
	    t = (driTMMImagePtr)images[i];
	    retcode = driTMMMakeImageResident(tmm, t, &addresses[i]);
	    if (t && t->area) t->area->locked = 1;
	}
    }

				/* Unlock resident images */
    for (i = 0; i < count; i++) {
	t = (driTMMImagePtr)images[i];
	if (t && t->area) t->area->locked = 0;
    }
    
    return retcode ? -1 : 0;
}


