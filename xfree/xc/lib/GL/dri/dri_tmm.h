/* dri_tmm.c -- High-level texture memory management
 * Created: Sun May 16 06:58:52 1999 by faith@precisioninsight.com
 * Revised: Mon May 17 22:04:50 1999 by faith@precisioninsight.com
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
 * $XFree86: xc/lib/GL/dri/dri_tmm.h,v 1.2 2000/02/23 04:46:34 martin Exp $
 * 
 */

#ifndef _DRI_TMM_H_
#define _DRI_TMM_H_

#define DRI_TMM_MAX_ORDER        32
#define DRI_TMM_DEFAULT_PRIORITY  0
#define DRI_TMM_PRIORITIES       10


typedef int (*driTMMLoadImage)(void *private,
			       unsigned long address,
			       int width,
			       int height,
			       int texel_width,
			       const unsigned char *image);

typedef int (*driTMMLoadSubImage)(void *private,
				  unsigned long address,
				  int width,
				  int height,
				  int texel_width,
				  int xoffset,
				  int yoffset,
				  int subimage_width,
				  int subimage_height,
				  const unsigned char *image);

typedef struct driTMMImage {
    int                 width;
    int                 height;
    int                 texel_width;
    unsigned char       *buffer;

    unsigned long       length;	/* Length of buffer                       */
    unsigned long       size;	/* Size of area required to hold buffer   */
    int                 priority; /* Priority                             */
    void                *private; /* Pointer to private area              */
    struct driTMMArea   *area;
} driTMMImage, *driTMMImagePtr;

typedef struct driTMMArea {
    unsigned long     offset;
    unsigned long     size;
    int               priority;
    driTMMImagePtr    image;
    int               locked;	/* Cannot be evicted          */
    struct driTMMArea *prev;	/* Previous area in LRU chain */
    struct driTMMArea *next;	/* Next area in LRU chain     */
} driTMMArea, *driTMMAreaPtr;

typedef struct driTMMPrio {
    driTMMAreaPtr     head;
    driTMMAreaPtr     tail;
} driTMMPrio, *driTMMPrioPtr;

typedef struct driTMM {
    unsigned long      start;
    unsigned long      current;
    unsigned long      end;
    unsigned long      size;
    unsigned long      word_size;
    unsigned long      alignment;
    driTMMLoadImage    load_image;
    driTMMLoadSubImage load_subimage;

    void               *freelist; /* Skip list ordered by address           */
    void               *images;   /* Hash table of driTMMImagePtr's         */
    driTMMPrio         priolist[DRI_TMM_PRIORITIES]; /* In use list         */
    int                priority;
    
    enum {
	DRI_TMM_FIRST_FIT,
	DRI_TMM_BEST_FIT
    }                  strategy;
    
				/* Statistics */
    unsigned long      hits;	/* Times already resident */
    unsigned long      misses;	/* Times not resident */
} driTMM, *driTMMPtr;

extern driTMMPtr     driTMMCreate(unsigned long      start,
				  unsigned long      size,
				  unsigned long      word_size, /* in bytes */
				  unsigned long      alignment,
				  driTMMLoadImage    load_image,
				  driTMMLoadSubImage load_subimage);
extern void          driTMMDestroy(driTMMPtr tmm);
extern void          *driTMMInsertImage(driTMMPtr           tmm,
					int                 width,
					int                 height,
					int                 texel_width,
					const unsigned char *image,
					void                *private);
extern int           driTMMSubImage(driTMMPtr           tmm,
				    void                *image,
				    int                 xoffset,
				    int                 yoffset,
				    int                 subimage_width,
				    int                 subimage_height,
				    const unsigned char *subimage);
extern int           driTMMDeleteImage(driTMMPtr tmm, void *image);
extern int           driTMMDeleteImages(driTMMPtr tmm,
					int count,
					void **images);
extern int           driTMMIsImageResident(driTMMPtr tmm, void *image);
extern int           driTMMAreImagesResident(driTMMPtr tmm,
					     int count,
					     void **images,
					     int *residences);
extern void          driTMMPrioritizeImage(driTMMPtr tmm,
					   void *image,
					   float priority);
extern void          driTMMPrioritizeImages(driTMMPtr tmm,
					    int count,
					    void **images,
					    float *priorities);
extern void          driTMMSetDefaultPriority(driTMMPtr tmm, float priority);
extern int           driTMMMakeImageResident(driTMMPtr tmm, void *image,
					     unsigned long *addresses);
extern int           driTMMMakeImagesResident(driTMMPtr tmm,
					      int count,
					      void **images,
					      unsigned long *addresses);
				    
#endif
