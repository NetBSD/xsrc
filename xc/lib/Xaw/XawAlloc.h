/* $XFree86: xc/lib/Xaw/XawAlloc.h,v 1.1.2.1 1998/05/16 09:05:23 dawes Exp $ */

#define XtStackAlloc(size, stack_cache_array)     \
    ((size) <= sizeof(stack_cache_array)          \
    ?  (XtPointer)(stack_cache_array)             \
    :  XtMalloc((unsigned)(size)))
     
#define XtStackFree(pointer, stack_cache_array) \
    if ((pointer) != ((XtPointer)(stack_cache_array))) XtFree(pointer); else

