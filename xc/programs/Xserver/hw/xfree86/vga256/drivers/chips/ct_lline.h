/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/chips/ct_lline.h,v 3.0 1996/08/11 13:02:51 dawes Exp $ */

/* ct_lline.c */

extern void ctcfb8LinearFramebufferVerticalLine(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int
#endif
);

extern void ctcfb16LinearFramebufferVerticalLine(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int
#endif
);

extern void ctcfb24LinearFramebufferVerticalLine(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int
#endif
);

extern void ctcfb8LinearFramebufferDualVerticalLine(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb16LinearFramebufferDualVerticalLine(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb24LinearFramebufferDualVerticalLine(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb8LinearFramebufferSlopedLineLeft(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb16LinearFramebufferSlopedLineLeft(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb24LinearFramebufferSlopedLineLeft(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb8LinearFramebufferSlopedLineRight(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb16LinearFramebufferSlopedLineRight(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb24LinearFramebufferSlopedLineRight(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb8LinearFramebufferSlopedLineVerticalLeft(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb16LinearFramebufferSlopedLineVerticalLeft(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb24LinearFramebufferSlopedLineVerticalLeft(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb8LinearFramebufferSlopedLineVerticalRight(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb16LinearFramebufferSlopedLineVerticalRight(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);

extern void ctcfb24LinearFramebufferSlopedLineVerticalRight(
#if NeedFunctionPrototypes
    unsigned char *,
    int,
    int,
    int,
    int,
    int,
    int
#endif
);
