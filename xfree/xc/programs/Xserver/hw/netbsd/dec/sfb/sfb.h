/*	$NetBSD: sfb.h,v 1.1 2004/01/18 05:21:41 rtr Exp $	*/

#include <sys/types.h>
#include <dev/tc/sfbreg.h>

#ifdef __alpha__
#define sfb_mb()    __asm__ __volatile__("mb" : : : "memory")
#elif defined __mips__
/* XXX */
#define	sfb_mb()
#else
#error No support for your architecture
#endif

/*
 * sfbline.c
 */

void	sfbSegmentSS(DrawablePtr, GCPtr, int, xSegment *);
void	sfbLineSS(DrawablePtr, GCPtr, int, int, DDXPointPtr);
void	sfbBresS(volatile u_int32_t *, int, int, int, int, int, int, int, int,
		 int, int, int);
void	sfbStraightS(volatile u_int32_t	*reg, int, int, int);

/*
 * XXX
 */

void 
decSfbCopyWindow(
#if NeedFunctionPrototypes
    WindowPtr /* pWin */,
    DDXPointRec /* ptOldOrg */,
    RegionPtr /* prgnSrc */
#endif
);

void 
decSfb32CopyWindow(
#if NeedFunctionPrototypes
    WindowPtr /* pWin */,
    DDXPointRec /* ptOldOrg */,
    RegionPtr /* prgnSrc */
#endif
);

Bool
decSfbCreateGC(
#if NeedFunctionPrototypes
    register GCPtr /* pGC */
#endif
);

Bool
decSfb32CreateGC(
#if NeedFunctionPrototypes
    register GCPtr /* pGC */
#endif
);

void
decSfbValidateGC(
#if NeedFunctionPrototypes
    register GCPtr  /* pGC */,
    unsigned long   /* changes */,
    DrawablePtr	    /* pDrawable */
#endif
);

void
decSfb32ValidateGC(
#if NeedFunctionPrototypes
    register GCPtr  /* pGC */,
    unsigned long   /* changes */,
    DrawablePtr	    /* pDrawable */
#endif
);

RegionPtr
decSfbCopyArea(
#if NeedFunctionPrototypes
    register DrawablePtr /* pSrcDrawable */,
    register DrawablePtr /* pDstDrawable */,
    GCPtr /* pGC */,
    int /* srcx */,
    int /* srcy */,
    int /* width */,
    int /* height */,
    int /* dstx */,
    int /* dsty */
#endif
);

RegionPtr
decSfb32CopyArea(
#if NeedFunctionPrototypes
    register DrawablePtr /* pSrcDrawable */,
    register DrawablePtr /* pDstDrawable */,
    GCPtr /* pGC */,
    int /* srcx */,
    int /* srcy */,
    int /* width */,
    int /* height */,
    int /* dstx */,
    int /* dsty */
#endif
);

void
decSfbFillSpans(
#if NeedFunctionPrototypes
    DrawablePtr /* pDrawable */,
    GCPtr	/* pGC */,
    int		/* nInit */,
    DDXPointPtr /* pptInit */,
    int*	/* pwidthInit */,
    int 	/* fSorted */
#endif
);

void
decSfb32FillSpans(
#if NeedFunctionPrototypes
    DrawablePtr /* pDrawable */,
    GCPtr	/* pGC */,
    int		/* nInit */,
    DDXPointPtr /* pptInit */,
    int*	/* pwidthInit */,
    int 	/* fSorted */
#endif
);
