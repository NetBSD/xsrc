/*	$NetBSD: tga.h,v 1.1 2004/01/18 05:21:41 rtr Exp $	*/

#include <sys/types.h>
#include <dev/pci/tgareg.h>

void 
decTgaCopyWindow(
#if NeedFunctionPrototypes
    WindowPtr /* pWin */,
    DDXPointRec /* ptOldOrg */,
    RegionPtr /* prgnSrc */
#endif
);

void 
decTga32CopyWindow(
#if NeedFunctionPrototypes
    WindowPtr /* pWin */,
    DDXPointRec /* ptOldOrg */,
    RegionPtr /* prgnSrc */
#endif
);

Bool
decTgaCreateGC(
#if NeedFunctionPrototypes
    register GCPtr /* pGC */
#endif
);

Bool
decTga32CreateGC(
#if NeedFunctionPrototypes
    register GCPtr /* pGC */
#endif
);

void
decTgaValidateGC(
#if NeedFunctionPrototypes
    register GCPtr  /* pGC */,
    unsigned long   /* changes */,
    DrawablePtr	    /* pDrawable */
#endif
);

void
decTga32ValidateGC(
#if NeedFunctionPrototypes
    register GCPtr  /* pGC */,
    unsigned long   /* changes */,
    DrawablePtr	    /* pDrawable */
#endif
);

RegionPtr
decTgaCopyArea(
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
decTga32CopyArea(
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
decTgaFillSpans(
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
decTga32FillSpans(
#if NeedFunctionPrototypes
    DrawablePtr /* pDrawable */,
    GCPtr	/* pGC */,
    int		/* nInit */,
    DDXPointPtr /* pptInit */,
    int*	/* pwidthInit */,
    int 	/* fSorted */
#endif
);
