/* $Xorg: miscutil.c,v 1.4 2001/02/09 02:04:04 xorgcvs Exp $ */

/*

Copyright 1991, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/font/util/miscutil.c,v 1.9 2005/01/11 23:01:12 tsi Exp $ */

#include <X11/Xosdefs.h>
#include <stdlib.h>
#include "fontmisc.h"

#define XK_LATIN1
#include    <X11/keysymdef.h>
/* #include    <X11/Xmu/CharSet.h> */

/* make sure everything initializes themselves at least once */

long serverGeneration = 1;

void *
Xalloc (unsigned long m)
{
#if defined(WORD64) || defined(LONG64)
    if (m & ~((unsigned long)(unsigned int)(-1))) return NULL;
#endif
    return malloc (m);
}

void *
Xrealloc (void *n, unsigned long m)
{
#if defined(WORD64) || defined(LONG64)
    if (m & ~((unsigned long)(unsigned int)(-1))) return NULL;
#endif
    if (!n)
	return malloc (m);
    else
	return realloc (n, m);
}

void *
Xcalloc (unsigned long m)
{
#if defined(WORD64) || defined(LONG64)
    if (m & ~((unsigned long)(unsigned int)(-1))) return NULL;
#endif
    return calloc (m, 1);
}

void
Xfree (void *n)
{
    if (n)
	free (n);
}

#if !defined(WORD64) && !defined(LONG64)

void *
Xllalloc(unsigned long long m)
{
    if (m & ~((unsigned long long)(unsigned long)(-1L))) return NULL;
    return Xalloc(m);
}

void *
Xllrealloc(void *n, unsigned long long m)
{
    if (m & ~((unsigned long long)(unsigned long)(-1L))) return NULL;
    return Xrealloc(n, m);
}

void *
Xllcalloc(unsigned long long m)
{
    if (m & ~((unsigned long long)(unsigned long)(-1L))) return NULL;
    return Xcalloc(m);
}

#endif

void
CopyISOLatin1Lowered (char *dst, char *src, int len)
{
    register unsigned char *dest, *source;

    for (dest = (unsigned char *)dst, source = (unsigned char *)src;
	 *source && len > 0;
	 source++, dest++, len--)
    {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}

void
register_fpe_functions ()
{
}
