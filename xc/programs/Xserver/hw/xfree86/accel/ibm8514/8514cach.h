/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/ibm8514/8514cach.h,v 3.3 1996/02/04 09:01:44 dawes Exp $ */





/* $XConsortium: 8514cach.h /main/2 1995/11/12 16:53:40 kaleb $ */

void ibm8514GlyphWrite(
#if NeedFunctionPrototypes
    int /*x*/,
    int /*y*/,
    int /*count*/,
    unsigned char */*chars*/,
    CacheFont8Ptr /*fentry*/,
    GCPtr /*pGC*/,
    BoxPtr /*pBox*/,
    int /*numRects*/
#endif
);

int ibm8514NoCPolyText(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char */*chars*/,
    Bool /*is8bit*/
#endif
);

int ibm8514NoCImageText(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char */*chars*/,
    Bool /*is8bit*/
#endif
);

void ibm8514CacheMoveBlock(
#if NeedFunctionPrototypes
    int /*srcx*/,
    int /*srcy*/,
    int /*dstx*/,
    int /*dsty*/,
    int /*h*/,
    int /*len*/,
    unsigned int /*id*/
#endif
);


void ibm8514FontOpStipple(
#if NeedFunctionPrototypes
    int /*x*/,
    int /*y*/,
    int /*w*/,
    int /*h*/,
    unsigned char */*psrc*/,
    int /*pwidth*/,
    Pixel /*id*/
#endif
);
