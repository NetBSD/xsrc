/* $XConsortium: xf86fcache.h,v 1.4 95/01/05 20:25:04 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/cache/xf86fcache.h,v 3.5 1995/01/28 16:57:44 dawes Exp $ */
/*
 * Data structures and function prototypes for the font cache.
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#ifndef _XF86_FCACHE_H
#define _XF86_FCACHE_H

/*
 * This struct contains all info for a cached font.
 */
typedef struct _CacheFont8 {
     FontPtr font;		       /* font */
     CharInfoPtr pci[256];	       /* font infos */
     short w;			       /* font cache spacing */
     short h;			       /* font max height */
     struct _bitMapBlock *fblock[8];   /* 8 * 32 cache block chars */
     struct _CacheFont8 *next;	       /* next */
}
CacheFont8Rec;

typedef struct _CacheFont8 *CacheFont8Ptr;

/*
 * Entrypoints into cache code.
 */

void xf86ReleaseFontCache();

void xf86InitFontCache(
#if NeedFunctionPrototypes
    CachePool /*FontCache*/,
    int /*RowWidth*/,
    int /*RowHeight*/,
    void (* /*FontOpStippleFunc*/)(
#if NeedNestedPrototypes
	int, int, int, int, unsigned char *, int, Pixel
#endif
    )
#endif
);

void xf86UnCacheFont8(
#if NeedFunctionPrototypes
    FontPtr /*font*/
#endif
);
     
CacheFont8Ptr xf86CacheFont8(
#if NeedFunctionPrototypes
    FontPtr /*font*/
#endif
);

void xf86loadFontBlock(
#if NeedFunctionPrototypes
    CacheFont8Ptr /*fentry*/,
    int /*block*/
#endif
);

#endif /* _XF86_FCACHE_H */
