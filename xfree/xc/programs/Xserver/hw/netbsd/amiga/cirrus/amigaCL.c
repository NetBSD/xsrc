
#ifndef lint
static char *rid="$XConsortium: amigaCL.c,v 1.26 94/04/17 20:29:38 kaleb Exp $";
#endif /* lint */
/*
Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include        "amiga.h"

#include        "Xmd.h"
#include        "gcstruct.h"
#include        "scrnintstr.h"
#include        "pixmapstr.h"
#include        "regionstr.h"
#include        "mistruct.h"
#include        "mifillarc.h"
#include        "fontstruct.h"
#include        "dixfontstr.h"
#include        "cfb.h"
#include        "cfbmskbits.h"
#include        "cfb8bit.h"
#include        "fastblt.h"
#include        "mergerop.h"
#include        "amigaCL.h"
#include        "migc.h"


Bool
amigaCLGXInit(ScreenPtr pScreen, fbFd *fb)
{
#if 0
    if (serverGeneration != amigaCLGeneration)
    {
        amigaCLScreenPrivateIndex = AllocateScreenPrivateIndex();
        if (amigaCLScreenPrivateIndex == -1)
            return FALSE;
        amigaCLGCPrivateIndex = AllocateGCPrivateIndex ();
        amigaCLWindowPrivateIndex = AllocateWindowPrivateIndex ();
        amigaCLGeneration = serverGeneration;
    }

    if (!AllocateGCPrivate(pScreen, amigaCLGCPrivateIndex, sizeof (amigaCLPrivGC
Rec)))
        return FALSE;
    if (!AllocateWindowPrivate(pScreen, amigaCLWindowPrivateIndex, 0))
        return FALSE;
    /*
     * Replace various screen functions
     */
#endif
    if (fb->info.gd_planes == 8)
      {
        pScreen->CreateGC = clCreateGC;
	pScreen->CopyWindow = clCopyWindow;
      }
    return TRUE;
}
