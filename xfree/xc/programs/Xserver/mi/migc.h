/* $TOG: migc.h /main/5 1998/02/09 14:47:18 kaleb $ */
/*

Copyright 1993, 1998  The Open Group

All Rights Reserved.

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

/* $XFree86: xc/programs/Xserver/mi/migc.h,v 1.5 1998/10/04 09:39:28 dawes Exp $ */

extern void miChangeGC(
#if NeedFunctionPrototypes
    GCPtr  /*pGC*/,
    unsigned long /*mask*/
#endif
);

extern void miDestroyGC(
#if NeedFunctionPrototypes
    GCPtr  /*pGC*/
#endif
);

extern GCOpsPtr miCreateGCOps(
#if NeedFunctionPrototypes
    GCOpsPtr /*prototype*/
#endif
);

extern void miDestroyGCOps(
#if NeedFunctionPrototypes
    GCOpsPtr /*ops*/
#endif
);

extern void miDestroyClip(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/
#endif
);

extern void miChangeClip(
#if NeedFunctionPrototypes
    GCPtr   /*pGC*/,
    int     /*type*/,
    pointer /*pvalue*/,
    int     /*nrects*/
#endif
);

extern void miCopyClip(
#if NeedFunctionPrototypes
    GCPtr /*pgcDst*/,
    GCPtr /*pgcSrc*/
#endif
);

extern void miCopyGC(
#if NeedFunctionPrototypes
    GCPtr /*pGCSrc*/,
    unsigned long /*changes*/,
    GCPtr /*pGCDst*/
#endif
);

extern void miComputeCompositeClip(
#if NeedFunctionPrototypes
    GCPtr       /*pGC*/,
    DrawablePtr /*pDrawable*/
#endif
);
