/* $Xorg: ResourceI.h,v 1.3 2000/08/17 19:46:16 cpqbld Exp $ */

/***********************************************************

Copyright 1987, 1988, 1994, 1998  The Open Group

All Rights Reserved.

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


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/****************************************************************
 *
 * Resources
 *
 ****************************************************************/

#define StringToQuark(string) XrmStringToQuark(string)
#define StringToName(string) XrmStringToName(string)
#define StringToClass(string) XrmStringToClass(string)

extern void _XtResourceDependencies(
#if NeedFunctionPrototypes
    WidgetClass  /* wc */
#endif
);

extern void _XtConstraintResDependencies(
#if NeedFunctionPrototypes
    ConstraintWidgetClass  /* wc */
#endif
);

extern XtCacheRef* _XtGetResources(
#if NeedFunctionPrototypes
    Widget	    /* w */,
    ArgList	    /* args */,
    Cardinal	    /* num_args */,
    XtTypedArgList  /* typed_args */,
    Cardinal*	    /* num_typed_args */
#endif
);

extern void _XtCopyFromParent(
#if NeedFunctionPrototypes
    Widget		/* widget */,
    int			/* offset */,
    XrmValue*		/* value */
#endif
);
