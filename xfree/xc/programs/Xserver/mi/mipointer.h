/*
 * mipointer.h
 *
 */

/* $Xorg: mipointer.h,v 1.3 2000/08/17 19:53:38 cpqbld Exp $ */

/*

Copyright 1989, 1998  The Open Group

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
*/
/* $XFree86: xc/programs/Xserver/mi/mipointer.h,v 3.7 2001/03/25 05:32:15 tsi Exp $ */

#ifndef MIPOINTER_H
#define MIPOINTER_H

#include "cursor.h"
#include "input.h"

typedef struct _miPointerSpriteFuncRec {
    Bool	(*RealizeCursor)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
                    CursorPtr /* pCurs */
#endif
                    );
    Bool	(*UnrealizeCursor)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
                    CursorPtr /* pCurs */
#endif
                    );
    void	(*SetCursor)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
                    CursorPtr /* pCurs */,
                    int  /* x */,
                    int  /* y */
#endif
                    );
    void	(*MoveCursor)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
                    int  /* x */,
                    int  /* y */
#endif
                    );
} miPointerSpriteFuncRec, *miPointerSpriteFuncPtr;

typedef struct _miPointerScreenFuncRec {
    Bool	(*CursorOffScreen)(
#if NeedFunctionPrototypes
                    ScreenPtr* /* ppScr */,
                    int*  /* px */,
                    int*  /* py */
#endif
                    );
    void	(*CrossScreen)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
                    int  /* entering */
#endif
                    );
    void	(*WarpCursor)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
                    int  /* x */,
                    int  /* y */
#endif
                    );
    void	(*EnqueueEvent)(
#if NeedFunctionPrototypes
                    xEventPtr /* event */
#endif
                    );
    void	(*NewEventScreen)(
#if NeedFunctionPrototypes
                    ScreenPtr /* pScr */,
		    Bool /* fromDIX */
#endif
                    );
} miPointerScreenFuncRec, *miPointerScreenFuncPtr;

extern Bool miDCInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miPointerScreenFuncPtr /*screenFuncs*/
#endif
);

extern Bool miPointerInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miPointerSpriteFuncPtr /*spriteFuncs*/,
    miPointerScreenFuncPtr /*screenFuncs*/,
    Bool /*waitForUpdate*/
#endif
);

extern void miPointerWarpCursor(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int /*x*/,
    int /*y*/
#endif
);

extern int miPointerGetMotionBufferSize(
#if NeedFunctionPrototypes
    void
#endif
);

extern int miPointerGetMotionEvents(
#if NeedFunctionPrototypes
    DeviceIntPtr /*pPtr*/,
    xTimecoord * /*coords*/,
    unsigned long /*start*/,
    unsigned long /*stop*/,
    ScreenPtr /*pScreen*/
#endif
);

extern void miPointerUpdate(
#if NeedFunctionPrototypes
    void
#endif
);

extern void miPointerDeltaCursor(
#if NeedFunctionPrototypes
    int /*dx*/,
    int /*dy*/,
    unsigned long /*time*/
#endif
);

extern void miPointerAbsoluteCursor(
#if NeedFunctionPrototypes
    int /*x*/,
    int /*y*/,
    unsigned long /*time*/
#endif
);

extern void miPointerPosition(
#if NeedFunctionPrototypes
    int * /*x*/,
    int * /*y*/
#endif
);

#undef miRegisterPointerDevice
extern void miRegisterPointerDevice(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    DevicePtr /*pDevice*/
#endif
);

extern void miPointerSetNewScreen(
#if NeedFunctionPrototypes
    int, /*screen_no*/
	int, /*x*/
	int /*y*/
#endif
);
extern ScreenPtr miPointerCurrentScreen(
#if NeedFunctionPrototypes
    void
#endif
);

#define miRegisterPointerDevice(pScreen,pDevice) \
       _miRegisterPointerDevice(pScreen,pDevice)

extern void _miRegisterPointerDevice(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    DeviceIntPtr /*pDevice*/
#endif
);

extern int miPointerScreenIndex;

#endif /* MIPOINTER_H */
