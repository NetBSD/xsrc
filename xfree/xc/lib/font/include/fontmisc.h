/* $Xorg: fontmisc.h,v 1.4 2001/02/09 02:04:04 xorgcvs Exp $ */

/*

Copyright 1991, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/lib/font/include/fontmisc.h,v 3.20 2005/01/27 22:50:40 tsi Exp $ */

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _FONTMISC_H_
#define _FONTMISC_H_

#ifndef FONTMODULE
#include <X11/Xfuncs.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef X_NOT_POSIX
#include <unistd.h>
#else
extern int close();
#endif

#endif /* FONTMODULE */

#include <X11/Xdefs.h>
#include <X11/Xmd.h>

#ifndef LSBFirst
#define LSBFirst	0
#define MSBFirst	1
#endif

#ifndef None
#define None	0l
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

extern Atom MakeAtom ( char *string, unsigned len, int makeit );
extern int ValidAtom ( Atom atom );
extern char *NameForAtom (Atom atom);

extern int f_strcasecmp(const char *s1, const char *s2);

#ifndef _HAVE_XALLOC_DECLS
#define _HAVE_XALLOC_DECLS
extern pointer Xalloc(unsigned long);
extern pointer Xrealloc(pointer, unsigned long);
extern pointer Xcalloc(unsigned long);
#if !defined(WORD64) && !defined(LONG64)
extern pointer Xllalloc(unsigned long long);
extern pointer Xllrealloc(pointer, unsigned long long);
extern pointer Xllcalloc(unsigned long long);
#endif
extern void Xfree(pointer);
#endif

#ifndef xalloc
/* Note:  It's important here that we cast "n", not "(n)";  ditto for "s" */
#if defined(WORD64) || defined(LONG64)
#define xalloc(n)     Xalloc((unsigned long)n)
#define xrealloc(p,n) Xrealloc((pointer)p, (unsigned long)n)
#define xcalloc(n,s)  Xcalloc(((unsigned long)n) * \
                              ((unsigned long)s))
#else
#define xalloc(n)     Xllalloc((unsigned long long)n)
#define xrealloc(p,n) Xllrealloc((pointer)p, (unsigned long long)n)
#define xcalloc(n,s)  Xllcalloc(((unsigned long long)n) * \
                                 ((unsigned long long)s))
#endif
#define xfree(p)      Xfree((pointer)p)
#endif

#define lowbit(x) ((x) & (~(x) + 1))

#undef assert
#define assert(x)	((void)0)

#ifndef strcasecmp
#if defined(NEED_STRCASECMP) && !defined(FONTMODULE)
#define strcasecmp(s1,s2) f_strcasecmp(s1,s2)
#endif
#endif

extern void
BitOrderInvert(
    register unsigned char *,
    register int
);

extern void
TwoByteSwap(
    register unsigned char *,
    register int
);

extern void
FourByteSwap(
    register unsigned char *,
    register int
);

extern int
RepadBitmap (
    char*, 
    char*,
    unsigned, 
    unsigned,
    int, 
    int
);

extern void CopyISOLatin1Lowered(
    char * /*dest*/,
    char * /*source*/,
    int /*length*/
);

extern void register_fpe_functions(void);

#endif /* _FONTMISC_H_ */
