/* $TOG: HookObjI.h /main/3 1998/02/06 13:22:28 kaleb $ */

/*

Copyright 1994, 1998  The Open Group

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

#ifndef _XtHookObjI_h
#define _XtHookObjI_h

/* This object is implementation-dependent and private to the library. */

typedef struct _HookObjRec *HookObject;
typedef struct _HookObjClassRec *HookObjectClass;

externalref WidgetClass hookObjectClass;

typedef struct _HookObjPart {
    /* resources */
    XtCallbackList createhook_callbacks;
    XtCallbackList changehook_callbacks;
    XtCallbackList confighook_callbacks;
    XtCallbackList geometryhook_callbacks;
    XtCallbackList destroyhook_callbacks;
    WidgetList shells;
    Cardinal num_shells;
    /* private data */
    Cardinal max_shells;
    Screen* screen;
}HookObjPart;

typedef struct _HookObjRec {
    ObjectPart object;
    HookObjPart hooks;
} HookObjRec;

typedef struct _HookObjClassPart {
    int unused;
} HookObjClassPart;

typedef struct _HookObjClassRec {
    ObjectClassPart object_class;
    HookObjClassPart hook_class;
} HookObjClassRec;

externalref HookObjClassRec hookObjClassRec;

#endif /* ifndef _Xt_HookObjI_h */


