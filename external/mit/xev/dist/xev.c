/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlocale.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/extensions/Xrandr.h>

#define INNER_WINDOW_WIDTH 50
#define INNER_WINDOW_HEIGHT 50
#define INNER_WINDOW_BORDER 4
#define INNER_WINDOW_X 10
#define INNER_WINDOW_Y 10
#define OUTER_WINDOW_MIN_WIDTH (INNER_WINDOW_WIDTH + \
				2 * (INNER_WINDOW_BORDER + INNER_WINDOW_X))
#define OUTER_WINDOW_MIN_HEIGHT (INNER_WINDOW_HEIGHT + \
				2 * (INNER_WINDOW_BORDER + INNER_WINDOW_Y))
#define OUTER_WINDOW_DEF_WIDTH (OUTER_WINDOW_MIN_WIDTH + 100)
#define OUTER_WINDOW_DEF_HEIGHT (OUTER_WINDOW_MIN_HEIGHT + 100)
#define OUTER_WINDOW_DEF_X 100
#define OUTER_WINDOW_DEF_Y 100


typedef unsigned long Pixel;

const char *Yes = "YES";
const char *No = "NO";
const char *Unknown = "unknown";

const char *ProgramName;
Display *dpy;
int screen;

XIC xic = NULL;

Atom wm_delete_window;
Atom wm_protocols;

Bool have_rr;
int rr_event_base, rr_error_base;

Bool single_line = False;

enum EventMaskIndex {
    EVENT_MASK_INDEX_CORE,
    EVENT_MASK_INDEX_RANDR,
    NUM_EVENT_MASKS
};

enum OutputFlags {
    InitialNewLine = 1,
    Indent         = 2,
    NewLine        = 4,
};

static void usage(const char *errmsg) _X_NORETURN;

static void
output_new_line(void)
{
    if (!single_line) {
        printf("\n");
    }
}

static void
_X_ATTRIBUTE_PRINTF(2, 3)
output(enum OutputFlags flags, const char* format, ...)
{
    va_list args;

    if (flags & InitialNewLine) {
        output_new_line();
    }
    if (flags & Indent) {
        printf(single_line ? " " : "    ");
    }

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    if (flags & NewLine) {
        output_new_line();
    }
}

static void
graceful_exit(int status)
{
    if (single_line) {
        printf("\n");
    }
    fflush(stdout);
    exit(status);
}

static void
prologue(XEvent *eventp, const char *event_name)
{
    XAnyEvent *e = (XAnyEvent *) eventp;

    output(InitialNewLine | NewLine,
           "%s event, serial %ld, synthetic %s, window 0x%lx,",
           event_name, e->serial, e->send_event ? Yes : No, e->window);
}

static void
dump(char *str, int len)
{
    output(0, "(");
    len--;
    while (len-- > 0)
        output(0, "%02x ", (unsigned char) *str++);
    output(0, "%02x)", (unsigned char) *str++);
}

static void
do_KeyPress(XEvent *eventp)
{
    XKeyEvent *e = (XKeyEvent *) eventp;
    KeySym ks;
    KeyCode kc = 0;
    Bool kc_set = False;
    const char *ksname;
    int nbytes, nmbbytes = 0;
    char str[256 + 1];
    static char *buf = NULL;
    static int bsize = 8;
    Status status;

    if (buf == NULL)
        buf = malloc(bsize);

    nbytes = XLookupString(e, str, 256, &ks, NULL);

    /* not supposed to call XmbLookupString on a key release event */
    if (e->type == KeyPress && xic) {
        do {
            nmbbytes = XmbLookupString(xic, e, buf, bsize - 1, &ks, &status);
            buf[nmbbytes] = '\0';

            if (status == XBufferOverflow) {
                bsize = nmbbytes + 1;
                buf = realloc(buf, bsize);
            }
        } while (status == XBufferOverflow);
    }

    if (ks == NoSymbol)
        ksname = "NoSymbol";
    else {
        if (!(ksname = XKeysymToString(ks)))
            ksname = "(no name)";
        kc = XKeysymToKeycode(dpy, ks);
        kc_set = True;
    }

    output(Indent | NewLine,
           "root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),",
           e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    output(Indent | NewLine,
           "state 0x%x, keycode %u (keysym 0x%lx, %s), same_screen %s,",
           e->state, e->keycode, (unsigned long) ks, ksname,
           e->same_screen ? Yes : No);
    if (kc_set && e->keycode != kc)
        output(Indent | NewLine, "XKeysymToKeycode returns keycode: %u", kc);
    if (nbytes < 0)
        nbytes = 0;
    if (nbytes > 256)
        nbytes = 256;
    str[nbytes] = '\0';
    output(Indent, "XLookupString gives %d bytes: ", nbytes);
    if (nbytes > 0) {
        dump(str, nbytes);
        output(NewLine, " \"%s\"", str);
    }
    else {
        output_new_line();
    }

    /* not supposed to call XmbLookupString on a key release event */
    if (e->type == KeyPress && xic) {
        output(Indent, "XmbLookupString gives %d bytes: ", nmbbytes);
        if (nmbbytes > 0) {
            dump(buf, nmbbytes);
            output(NewLine, " \"%s\"", buf);
        }
        else {
            output_new_line();
        }
    }

    output(Indent | NewLine, "XFilterEvent returns: %s",
           XFilterEvent(eventp, e->window) ? "True" : "False");
}

static void
do_KeyRelease(XEvent *eventp)
{
    do_KeyPress(eventp);        /* since it has the same info */
}

static void
do_ButtonPress(XEvent *eventp)
{
    XButtonEvent *e = (XButtonEvent *) eventp;

    output(Indent | NewLine,
           "root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),",
           e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    output(Indent | NewLine, "state 0x%x, button %u, same_screen %s",
           e->state, e->button, e->same_screen ? Yes : No);
}

static void
do_ButtonRelease(XEvent *eventp)
{
    do_ButtonPress(eventp);     /* since it has the same info */
}

static void
do_MotionNotify(XEvent *eventp)
{
    XMotionEvent *e = (XMotionEvent *) eventp;

    output(Indent | NewLine,
           "root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),",
           e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    output(Indent | NewLine, "state 0x%x, is_hint %u, same_screen %s",
           e->state, e->is_hint, e->same_screen ? Yes : No);
}

static void
do_EnterNotify(XEvent *eventp)
{
    XCrossingEvent *e = (XCrossingEvent *) eventp;
    const char *mode, *detail;
    char dmode[10], ddetail[10];

    switch (e->mode) {
    case NotifyNormal:
        mode = "NotifyNormal";
        break;
    case NotifyGrab:
        mode = "NotifyGrab";
        break;
    case NotifyUngrab:
        mode = "NotifyUngrab";
        break;
    case NotifyWhileGrabbed:
        mode = "NotifyWhileGrabbed";
        break;
    default:
        mode = dmode;
        snprintf(dmode, sizeof(dmode), "%u", e->mode);
        break;
    }

    switch (e->detail) {
    case NotifyAncestor:
        detail = "NotifyAncestor";
        break;
    case NotifyVirtual:
        detail = "NotifyVirtual";
        break;
    case NotifyInferior:
        detail = "NotifyInferior";
        break;
    case NotifyNonlinear:
        detail = "NotifyNonlinear";
        break;
    case NotifyNonlinearVirtual:
        detail = "NotifyNonlinearVirtual";
        break;
    case NotifyPointer:
        detail = "NotifyPointer";
        break;
    case NotifyPointerRoot:
        detail = "NotifyPointerRoot";
        break;
    case NotifyDetailNone:
        detail = "NotifyDetailNone";
        break;
    default:
        detail = ddetail;
        snprintf(ddetail, sizeof(ddetail), "%u", e->detail);
        break;
    }

    output(Indent | NewLine,
           "root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),",
           e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    output(Indent | NewLine, "mode %s, detail %s, same_screen %s,",
           mode, detail, e->same_screen ? Yes : No);
    output(Indent | NewLine, "focus %s, state %u",
           e->focus ? Yes : No, e->state);
}

static void
do_LeaveNotify(XEvent *eventp)
{
    do_EnterNotify(eventp);     /* since it has same information */
}

static void
do_FocusIn(XEvent *eventp)
{
    XFocusChangeEvent *e = (XFocusChangeEvent *) eventp;
    const char *mode, *detail;
    char dmode[10], ddetail[10];

    switch (e->mode) {
    case NotifyNormal:
        mode = "NotifyNormal";
        break;
    case NotifyGrab:
        mode = "NotifyGrab";
        break;
    case NotifyUngrab:
        mode = "NotifyUngrab";
        break;
    case NotifyWhileGrabbed:
        mode = "NotifyWhileGrabbed";
        break;
    default:
        mode = dmode;
        snprintf(dmode, sizeof(dmode), "%u", e->mode);
        break;
    }

    switch (e->detail) {
    case NotifyAncestor:
        detail = "NotifyAncestor";
        break;
    case NotifyVirtual:
        detail = "NotifyVirtual";
        break;
    case NotifyInferior:
        detail = "NotifyInferior";
        break;
    case NotifyNonlinear:
        detail = "NotifyNonlinear";
        break;
    case NotifyNonlinearVirtual:
        detail = "NotifyNonlinearVirtual";
        break;
    case NotifyPointer:
        detail = "NotifyPointer";
        break;
    case NotifyPointerRoot:
        detail = "NotifyPointerRoot";
        break;
    case NotifyDetailNone:
        detail = "NotifyDetailNone";
        break;
    default:
        detail = ddetail;
        snprintf(ddetail, sizeof(ddetail), "%u", e->detail);
        break;
    }

    output(Indent | NewLine, "mode %s, detail %s", mode, detail);
}

static void
do_FocusOut(XEvent *eventp)
{
    do_FocusIn(eventp);         /* since it has same information */
}

static void
do_KeymapNotify(XEvent *eventp)
{
    XKeymapEvent *e = (XKeymapEvent *) eventp;
    int i;

    output(Indent, "keys:  ");
    for (i = 0; i < 32; i++) {
        if (i == 16 && !single_line) {
            output(InitialNewLine | Indent, "       ");
        }
        output(0, "%-3u ", (unsigned char) e->key_vector[i]);
    }
    output_new_line();
}

static void
do_Expose(XEvent *eventp)
{
    XExposeEvent *e = (XExposeEvent *) eventp;

    output(Indent | NewLine, "(%d,%d), width %d, height %d, count %d",
           e->x, e->y, e->width, e->height, e->count);
}

static void
do_GraphicsExpose(XEvent *eventp)
{
    XGraphicsExposeEvent *e = (XGraphicsExposeEvent *) eventp;
    const char *m;
    char mdummy[10];

    switch (e->major_code) {
    case X_CopyArea:
        m = "CopyArea";
        break;
    case X_CopyPlane:
        m = "CopyPlane";
        break;
    default:
        m = mdummy;
        snprintf(mdummy, sizeof(mdummy), "%d", e->major_code);
        break;
    }

    output(Indent | NewLine, "(%d,%d), width %d, height %d, count %d,",
           e->x, e->y, e->width, e->height, e->count);
    output(Indent | NewLine, "major %s, minor %d", m, e->minor_code);
}

static void
do_NoExpose(XEvent *eventp)
{
    XNoExposeEvent *e = (XNoExposeEvent *) eventp;
    const char *m;
    char mdummy[10];

    switch (e->major_code) {
    case X_CopyArea:
        m = "CopyArea";
        break;
    case X_CopyPlane:
        m = "CopyPlane";
        break;
    default:
        m = mdummy;
        snprintf(mdummy, sizeof(mdummy), "%d", e->major_code);
        break;
    }

    output(Indent | NewLine, "major %s, minor %d", m, e->minor_code);
    return;
}

static void
do_VisibilityNotify(XEvent *eventp)
{
    XVisibilityEvent *e = (XVisibilityEvent *) eventp;
    const char *v;
    char vdummy[10];

    switch (e->state) {
    case VisibilityUnobscured:
        v = "VisibilityUnobscured";
        break;
    case VisibilityPartiallyObscured:
        v = "VisibilityPartiallyObscured";
        break;
    case VisibilityFullyObscured:
        v = "VisibilityFullyObscured";
        break;
    default:
        v = vdummy;
        snprintf(vdummy, sizeof(vdummy), "%d", e->state);
        break;
    }

    output(Indent | NewLine, "state %s", v);
}

static void
do_CreateNotify(XEvent *eventp)
{
    XCreateWindowEvent *e = (XCreateWindowEvent *) eventp;

    output(Indent | NewLine,
           "parent 0x%lx, window 0x%lx, (%d,%d), width %d, height %d",
           e->parent, e->window, e->x, e->y, e->width, e->height);
    output(NewLine, "border_width %d, override %s",
           e->border_width, e->override_redirect ? Yes : No);
}

static void
do_DestroyNotify(XEvent *eventp)
{
    XDestroyWindowEvent *e = (XDestroyWindowEvent *) eventp;

    output(Indent | NewLine, "event 0x%lx, window 0x%lx", e->event, e->window);
}

static void
do_UnmapNotify(XEvent *eventp)
{
    XUnmapEvent *e = (XUnmapEvent *) eventp;

    output(Indent | NewLine, "event 0x%lx, window 0x%lx, from_configure %s",
           e->event, e->window, e->from_configure ? Yes : No);
}

static void
do_MapNotify(XEvent *eventp)
{
    XMapEvent *e = (XMapEvent *) eventp;

    output(Indent | NewLine, "event 0x%lx, window 0x%lx, override %s",
           e->event, e->window, e->override_redirect ? Yes : No);
}

static void
do_MapRequest(XEvent *eventp)
{
    XMapRequestEvent *e = (XMapRequestEvent *) eventp;

    output(Indent | NewLine, "parent 0x%lx, window 0x%lx",
           e->parent, e->window);
}

static void
do_ReparentNotify(XEvent *eventp)
{
    XReparentEvent *e = (XReparentEvent *) eventp;

    output(Indent | NewLine, "event 0x%lx, window 0x%lx, parent 0x%lx,",
           e->event, e->window, e->parent);
    output(Indent | NewLine, "(%d,%d), override %s", e->x, e->y,
           e->override_redirect ? Yes : No);
}

static void
do_ConfigureNotify(XEvent *eventp)
{
    XConfigureEvent *e = (XConfigureEvent *) eventp;

    output(Indent | NewLine,
           "event 0x%lx, window 0x%lx, (%d,%d), width %d, height %d,",
           e->event, e->window, e->x, e->y, e->width, e->height);
    output(Indent | NewLine, "border_width %d, above 0x%lx, override %s",
           e->border_width, e->above, e->override_redirect ? Yes : No);
}

static void
do_ConfigureRequest(XEvent *eventp)
{
    XConfigureRequestEvent *e = (XConfigureRequestEvent *) eventp;
    const char *detail;
    char ddummy[10];

    switch (e->detail) {
    case Above:
        detail = "Above";
        break;
    case Below:
        detail = "Below";
        break;
    case TopIf:
        detail = "TopIf";
        break;
    case BottomIf:
        detail = "BottomIf";
        break;
    case Opposite:
        detail = "Opposite";
        break;
    default:
        detail = ddummy;
        snprintf(ddummy, sizeof(ddummy), "%d", e->detail);
        break;
    }

    output(Indent | NewLine,
           "parent 0x%lx, window 0x%lx, (%d,%d), width %d, height %d,",
           e->parent, e->window, e->x, e->y, e->width, e->height);
    output(Indent | NewLine,
           "border_width %d, above 0x%lx, detail %s, value 0x%lx",
           e->border_width, e->above, detail, e->value_mask);
}

static void
do_GravityNotify(XEvent *eventp)
{
    XGravityEvent *e = (XGravityEvent *) eventp;

    output(Indent | NewLine, "event 0x%lx, window 0x%lx, (%d,%d)",
           e->event, e->window, e->x, e->y);
}

static void
do_ResizeRequest(XEvent *eventp)
{
    XResizeRequestEvent *e = (XResizeRequestEvent *) eventp;

    output(Indent | NewLine, "width %d, height %d", e->width, e->height);
}

static void
do_CirculateNotify(XEvent *eventp)
{
    XCirculateEvent *e = (XCirculateEvent *) eventp;
    const char *p;
    char pdummy[10];

    switch (e->place) {
    case PlaceOnTop:
        p = "PlaceOnTop";
        break;
    case PlaceOnBottom:
        p = "PlaceOnBottom";
        break;
    default:
        p = pdummy;
        snprintf(pdummy, sizeof(pdummy), "%d", e->place);
        break;
    }

    output(Indent | NewLine, "event 0x%lx, window 0x%lx, place %s",
           e->event, e->window, p);
}

static void
do_CirculateRequest(XEvent *eventp)
{
    XCirculateRequestEvent *e = (XCirculateRequestEvent *) eventp;
    const char *p;
    char pdummy[10];

    switch (e->place) {
    case PlaceOnTop:
        p = "PlaceOnTop";
        break;
    case PlaceOnBottom:
        p = "PlaceOnBottom";
        break;
    default:
        p = pdummy;
        snprintf(pdummy, sizeof(pdummy), "%d", e->place);
        break;
    }

    output(Indent | NewLine, "parent 0x%lx, window 0x%lx, place %s",
           e->parent, e->window, p);
}

static void
do_PropertyNotify(XEvent *eventp)
{
    XPropertyEvent *e = (XPropertyEvent *) eventp;
    char *aname = XGetAtomName(dpy, e->atom);
    const char *s;
    char sdummy[10];

    switch (e->state) {
    case PropertyNewValue:
        s = "PropertyNewValue";
        break;
    case PropertyDelete:
        s = "PropertyDelete";
        break;
    default:
        s = sdummy;
        snprintf(sdummy, sizeof(sdummy), "%d", e->state);
        break;
    }

    output(Indent | NewLine, "atom 0x%lx (%s), time %lu, state %s",
           e->atom, aname ? aname : Unknown, e->time, s);

    XFree(aname);
}

static void
do_SelectionClear(XEvent *eventp)
{
    XSelectionClearEvent *e = (XSelectionClearEvent *) eventp;
    char *sname = XGetAtomName(dpy, e->selection);

    output(Indent | NewLine, "selection 0x%lx (%s), time %lu",
           e->selection, sname ? sname : Unknown, e->time);

    XFree(sname);
}

static void
do_SelectionRequest(XEvent *eventp)
{
    XSelectionRequestEvent *e = (XSelectionRequestEvent *) eventp;
    char *sname = XGetAtomName(dpy, e->selection);
    char *tname = XGetAtomName(dpy, e->target);
    char *pname = XGetAtomName(dpy, e->property);

    output(Indent | NewLine,
           "owner 0x%lx, requestor 0x%lx, selection 0x%lx (%s),",
           e->owner, e->requestor, e->selection, sname ? sname : Unknown);
    output(Indent | NewLine,
           "target 0x%lx (%s), property 0x%lx (%s), time %lu",
           e->target, tname ? tname : Unknown, e->property,
           pname ? pname : Unknown, e->time);

    XFree(sname);
    XFree(tname);
    XFree(pname);
}

static void
do_SelectionNotify(XEvent *eventp)
{
    XSelectionEvent *e = (XSelectionEvent *) eventp;
    char *sname = XGetAtomName(dpy, e->selection);
    char *tname = XGetAtomName(dpy, e->target);
    char *pname = XGetAtomName(dpy, e->property);

    output(Indent | NewLine, "selection 0x%lx (%s), target 0x%lx (%s),",
           e->selection, sname ? sname : Unknown, e->target,
           tname ? tname : Unknown);
    output(Indent | NewLine, "property 0x%lx (%s), time %lu",
           e->property, pname ? pname : Unknown, e->time);

    XFree(sname);
    XFree(tname);
    XFree(pname);
}

static void
do_ColormapNotify(XEvent *eventp)
{
    XColormapEvent *e = (XColormapEvent *) eventp;
    const char *s;
    char sdummy[10];

    switch (e->state) {
    case ColormapInstalled:
        s = "ColormapInstalled";
        break;
    case ColormapUninstalled:
        s = "ColormapUninstalled";
        break;
    default:
        s = sdummy;
        snprintf(sdummy, sizeof(sdummy), "%d", e->state);
        break;
    }

    output(Indent | NewLine, "colormap 0x%lx, new %s, state %s",
           e->colormap, e->new ? Yes : No, s);
}

static void
do_ClientMessage(XEvent *eventp)
{
    XClientMessageEvent *e = (XClientMessageEvent *) eventp;

    char *mname = XGetAtomName(dpy, e->message_type);

    if (e->message_type == wm_protocols) {
        char *message = XGetAtomName(dpy, e->data.l[0]);

        output(Indent | NewLine,
               "message_type 0x%lx (%s), format %d, message 0x%lx (%s)",
               e->message_type, mname ? mname : Unknown, e->format,
               e->data.l[0], message);
        XFree(message);
    }
    else {
        output(Indent | NewLine, "message_type 0x%lx (%s), format %d",
               e->message_type, mname ? mname : Unknown, e->format);
    }

    XFree(mname);

    if (e->format == 32
        && e->message_type == wm_protocols
        && (Atom) e->data.l[0] == wm_delete_window
    ) {
        graceful_exit(0);
    }
}

static void
do_MappingNotify(XEvent *eventp)
{
    XMappingEvent *e = (XMappingEvent *) eventp;
    const char *r;
    char rdummy[10];

    switch (e->request) {
    case MappingModifier:
        r = "MappingModifier";
        break;
    case MappingKeyboard:
        r = "MappingKeyboard";
        break;
    case MappingPointer:
        r = "MappingPointer";
        break;
    default:
        r = rdummy;
        snprintf(rdummy, sizeof(rdummy), "%d", e->request);
        break;
    }

    output(Indent | NewLine, "request %s, first_keycode %d, count %d",
           r, e->first_keycode, e->count);
    XRefreshKeyboardMapping(e);
}

static void
print_SubPixelOrder(SubpixelOrder subpixel_order)
{
    switch (subpixel_order) {
    case SubPixelUnknown:
        output(0, "SubPixelUnknown");
        return;
    case SubPixelHorizontalRGB:
        output(0, "SubPixelHorizontalRGB");
        return;
    case SubPixelHorizontalBGR:
        output(0, "SubPixelHorizontalBGR");
        return;
    case SubPixelVerticalRGB:
        output(0, "SubPixelVerticalRGB");
        return;
    case SubPixelVerticalBGR:
        output(0, "SubPixelVerticalBGR");
        return;
    case SubPixelNone:
        output(0, "SubPixelNone");
        return;
    default:
        output(0, "%d", subpixel_order);
    }
}

static void
print_Rotation(Rotation rotation)
{
    if (rotation & RR_Rotate_0)
        output(0, "RR_Rotate_0");
    else if (rotation & RR_Rotate_90)
        output(0, "RR_Rotate_90");
    else if (rotation & RR_Rotate_180)
        output(0, "RR_Rotate_180");
    else if (rotation & RR_Rotate_270)
        output(0, "RR_Rotate_270");
    else {
        output(0, "%d", rotation);
        return;
    }
    if (rotation & RR_Reflect_X)
        output(0, ", RR_Reflect_X");
    if (rotation & RR_Reflect_Y)
        output(0, ", RR_Reflect_Y");
}

static void
print_Connection(Connection connection)
{
    switch (connection) {
    case RR_Connected:
        output(0, "RR_Connected");
        return;
    case RR_Disconnected:
        output(0, "RR_Disconnected");
        return;
    case RR_UnknownConnection:
        output(0, "RR_UnknownConnection");
        return;
    default:
        output(0, "%d", connection);
    }
}

static void
do_RRScreenChangeNotify(XEvent *eventp)
{
    XRRScreenChangeNotifyEvent *e = (XRRScreenChangeNotifyEvent *) eventp;

    XRRUpdateConfiguration(eventp);
    output(Indent | NewLine, "root 0x%lx, timestamp %lu, config_timestamp %lu",
           e->root, e->timestamp, e->config_timestamp);
    output(Indent, "size_index %hu", e->size_index);
    output(0, ", subpixel_order ");
    print_SubPixelOrder(e->subpixel_order);
    output(InitialNewLine | Indent, "rotation ");
    print_Rotation(e->rotation);
    output(InitialNewLine | Indent | NewLine,
           "width %d, height %d, mwidth %d, mheight %d",
           e->width, e->height, e->mwidth, e->mheight);
}

static void
do_RRNotify_OutputChange(XEvent *eventp, XRRScreenResources *screen_resources)
{
    XRROutputChangeNotifyEvent *e = (XRROutputChangeNotifyEvent *) eventp;
    XRROutputInfo *output_info = NULL;
    XRRModeInfo *mode_info = NULL;

    if (screen_resources) {
        int i;

        output_info = XRRGetOutputInfo(dpy, screen_resources, e->output);
        for (i = 0; i < screen_resources->nmode; i++)
            if (screen_resources->modes[i].id == e->mode) {
                mode_info = &screen_resources->modes[i];
                break;
            }
    }
    output(Indent | NewLine, "subtype XRROutputChangeNotifyEvent");
    if (output_info)
        output(Indent, "output %s, ", output_info->name);
    else
        output(Indent, "output %lu, ", e->output);
    if (e->crtc)
        output(0, "crtc %lu, ", e->crtc);
    else
        output(0, "crtc None, ");
    if (mode_info)
        output(NewLine, "mode %s (%dx%d)", mode_info->name, mode_info->width,
               mode_info->height);
    else if (e->mode)
        output(NewLine, "mode %lu", e->mode);
    else
        output(NewLine, "mode None");
    output(Indent, "rotation ");
    print_Rotation(e->rotation);
    output(InitialNewLine | Indent, "connection ");
    print_Connection(e->connection);
    output(0, ", subpixel_order ");
    print_SubPixelOrder(e->subpixel_order);
    output_new_line();
    XRRFreeOutputInfo(output_info);
}

static void
do_RRNotify_CrtcChange(XEvent *eventp, XRRScreenResources *screen_resources)
{
    XRRCrtcChangeNotifyEvent *e = (XRRCrtcChangeNotifyEvent *) eventp;
    XRRModeInfo *mode_info = NULL;

    if (screen_resources) {
        int i;

        for (i = 0; i < screen_resources->nmode; i++)
            if (screen_resources->modes[i].id == e->mode) {
                mode_info = &screen_resources->modes[i];
                break;
            }
    }
    output(Indent | NewLine, "subtype XRRCrtcChangeNotifyEvent");
    if (e->crtc)
        output(Indent, "crtc %lu, ", e->crtc);
    else
        output(Indent, "crtc None, ");
    if (mode_info)
        output(0, "mode %s, ", mode_info->name);
    else if (e->mode)
        output(0, "mode %lu, ", e->mode);
    else
        output(0, "mode None, ");
    output(0, "rotation ");
    print_Rotation(e->rotation);
    output(InitialNewLine | Indent | NewLine, "x %d, y %d, width %d, height %d",
           e->x, e->y, e->width, e->height);
}

static void
do_RRNotify_OutputProperty(XEvent *eventp,
                           XRRScreenResources *screen_resources)
{
    XRROutputPropertyNotifyEvent *e = (XRROutputPropertyNotifyEvent *) eventp;
    XRROutputInfo *output_info = NULL;
    char *property = XGetAtomName(dpy, e->property);

    if (screen_resources)
        output_info = XRRGetOutputInfo(dpy, screen_resources, e->output);
    output(Indent | NewLine, "subtype XRROutputPropertyChangeNotifyEvent");
    if (output_info)
        output(Indent, "output %s, ", output_info->name);
    else
        output(Indent, "output %lu, ", e->output);
    output(0, "property %s, timestamp %lu, state ", property, e->timestamp);
    if (e->state == PropertyNewValue)
        output(NewLine, "NewValue");
    else if (e->state == PropertyDelete)
        output(NewLine, "Delete");
    else
        output(NewLine, "%d", e->state);
    XRRFreeOutputInfo(output_info);
    XFree(property);
}

static void
do_RRNotify(XEvent *eventp)
{
    XRRNotifyEvent *e = (XRRNotifyEvent *) eventp;
    XRRScreenResources *screen_resources;

    XRRUpdateConfiguration(eventp);
    screen_resources = XRRGetScreenResources(dpy, e->window);
    prologue(eventp, "RRNotify");
    switch (e->subtype) {
    case RRNotify_OutputChange:
        do_RRNotify_OutputChange(eventp, screen_resources);
        break;
    case RRNotify_CrtcChange:
        do_RRNotify_CrtcChange(eventp, screen_resources);
        break;
    case RRNotify_OutputProperty:
        do_RRNotify_OutputProperty(eventp, screen_resources);
        break;
    default:
        output(Indent | NewLine, "subtype %d", e->subtype);
    }
    XRRFreeScreenResources(screen_resources);
}

static void
set_sizehints(XSizeHints *hintp, int min_width, int min_height,
              int defwidth, int defheight, int defx, int defy, char *geom)
{
    int geom_result;

    /* set the size hints, algorithm from xlib xbiff */

    hintp->width = hintp->min_width = min_width;
    hintp->height = hintp->min_height = min_height;
    hintp->flags = PMinSize;
    hintp->x = hintp->y = 0;
    geom_result = NoValue;
    if (geom != NULL) {
        geom_result = XParseGeometry(geom, &hintp->x, &hintp->y,
                                     (unsigned int *) &hintp->width,
                                     (unsigned int *) &hintp->height);
        if ((geom_result & WidthValue) && (geom_result & HeightValue)) {
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
            hintp->width = max(hintp->width, hintp->min_width);
            hintp->height = max(hintp->height, hintp->min_height);
            hintp->flags |= USSize;
        }
        if ((geom_result & XValue) && (geom_result & YValue)) {
            hintp->flags += USPosition;
        }
    }
    if (!(hintp->flags & USSize)) {
        hintp->width = defwidth;
        hintp->height = defheight;
        hintp->flags |= PSize;
    }
/*
    if (!(hintp->flags & USPosition)) {
	hintp->x = defx;
	hintp->y = defy;
	hintp->flags |= PPosition;
    }
 */
    if (geom_result & XNegative) {
        hintp->x = DisplayWidth(dpy, DefaultScreen(dpy)) + hintp->x -
            hintp->width;
    }
    if (geom_result & YNegative) {
        hintp->y = DisplayHeight(dpy, DefaultScreen(dpy)) + hintp->y -
            hintp->height;
    }
}

static void
usage(const char *errmsg)
{
    const char *msg =
"    -display displayname                X server to contact\n"
"    -geometry geom                      size and location of window\n"
"    -bw pixels                          border width in pixels\n"
"    -bs {NotUseful,WhenMapped,Always}   backingstore attribute\n"
"    -id windowid                        use existing window\n"
"    -root                               use root window\n"
"    -s                                  set save-unders attribute\n"
"    -name string                        window name\n"
"    -rv                                 reverse video\n"
"    -event event_mask                   select 'event_mask' events\n"
"           Supported event masks: keyboard mouse expose visibility structure\n"
"                                  substructure focus property colormap\n"
"                                  owner_grab_button randr button\n"
"           This option can be specified multiple times to select multiple\n"
"           event masks.\n"
"    -1                                  display only a single line per event\n"
"    -version                            print version and exit\n"
"\n";

    if (errmsg != NULL)
        fprintf(stderr, "%s: %s\n", ProgramName, errmsg);

    fprintf(stderr, "usage:  %s [-options ...]\n", ProgramName);
    fprintf(stderr, "where options include:\n");
    fputs(msg, stderr);

    exit(1);
}

static int
parse_backing_store(const char *s)
{
    size_t len = strlen(s);

    if (strncasecmp(s, "NotUseful", len) == 0)
        return (NotUseful);
    if (strncasecmp(s, "WhenMapped", len) == 0)
        return (WhenMapped);
    if (strncasecmp(s, "Always", len) == 0)
        return (Always);

    fprintf(stderr, "%s: unrecognized argument '%s' for -bs\n", ProgramName, s);
    usage(NULL);
}

static Bool
parse_event_mask(const char *s, long event_masks[])
{
    const struct {
        const char *name;
        enum EventMaskIndex mask_index;
        long mask;
    } events[] = {
        { "keyboard",
          EVENT_MASK_INDEX_CORE,
          KeyPressMask | KeyReleaseMask | KeymapStateMask },
        { "mouse",
          EVENT_MASK_INDEX_CORE,
          ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
          LeaveWindowMask | PointerMotionMask | Button1MotionMask |
          Button2MotionMask | Button3MotionMask | Button4MotionMask |
          Button5MotionMask | ButtonMotionMask },
        { "button",
          EVENT_MASK_INDEX_CORE,
          ButtonPressMask | ButtonReleaseMask },
        { "expose",
          EVENT_MASK_INDEX_CORE,
          ExposureMask },
        { "visibility",
          EVENT_MASK_INDEX_CORE,
          VisibilityChangeMask },
        { "structure",
          EVENT_MASK_INDEX_CORE,
          StructureNotifyMask },
        { "substructure",
          EVENT_MASK_INDEX_CORE,
          SubstructureNotifyMask | SubstructureRedirectMask },
        { "focus",
          EVENT_MASK_INDEX_CORE,
          FocusChangeMask },
        { "property",
          EVENT_MASK_INDEX_CORE,
          PropertyChangeMask },
        { "colormap",
          EVENT_MASK_INDEX_CORE,
          ColormapChangeMask },
        { "owner_grab_button",
          EVENT_MASK_INDEX_CORE,
          OwnerGrabButtonMask },
        { "randr",
          EVENT_MASK_INDEX_RANDR,
          RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask |
          RROutputChangeNotifyMask | RROutputPropertyNotifyMask },
        { NULL, 0, 0 }
    };
    int i;

    for (i = 0; events[i].name; i++) {
        if (!s || !strcmp(s, events[i].name)) {
            event_masks[events[i].mask_index] |= events[i].mask;
            if (s)
                return True;
        }
    }

    if (s != NULL)
        fprintf(stderr, "%s: unrecognized event mask '%s'\n", ProgramName, s);

    return False;
}

int
main(int argc, char **argv)
{
    char *displayname = NULL;
    char *geom = NULL;
    int i;
    XSizeHints hints;
    int borderwidth = 2;
    Window w, subw;
    XSetWindowAttributes attr;
    XWindowAttributes wattr;
    unsigned long mask = 0L;
    int done;
    const char *name = "Event Tester";
    Bool reverse = False;
    Bool use_root = False;
    unsigned long back, fore;
    XIM xim;
    XIMStyles *xim_styles;
    XIMStyle xim_style = 0;
    char *modifiers;
    char *imvalret;
    long event_masks[NUM_EVENT_MASKS];
    Bool event_mask_specified = False;

    ProgramName = argv[0];

    if (setlocale(LC_ALL, "") == NULL) {
        fprintf(stderr, "%s: warning: could not set default locale\n",
                ProgramName);
    }

    memset(event_masks, 0, sizeof(event_masks));

    w = 0;
    for (i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (arg[0] == '-') {
            switch (arg[1]) {
            case 'd':          /* -display host:dpy */
                if (++i >= argc)
                    usage("-display requires an argument");
                displayname = argv[i];
                continue;
            case 'g':          /* -geometry geom */
                if (++i >= argc)
                    usage("-geometry requires an argument");
                geom = argv[i];
                continue;
            case 'b':
                switch (arg[2]) {
                case 'w':      /* -bw pixels */
                    if (++i >= argc)
                        usage("-bw requires an argument");
                    borderwidth = atoi(argv[i]);
                    continue;
                case 's':      /* -bs type */
                    if (++i >= argc)
                        usage("-bs requires an argument");
                    attr.backing_store = parse_backing_store(argv[i]);
                    mask |= CWBackingStore;
                    continue;
                default:
                    goto unrecognized;
                }
            case 'i':          /* -id */
                if (++i >= argc)
                    usage("-id requires an argument");
                sscanf(argv[i], "0x%lx", &w);
                if (!w)
                    sscanf(argv[i], "%lu", &w);
                if (!w) {
                    fprintf(stderr,
                            "%s: unable to parse argument '%s' for -id\n",
                            ProgramName, argv[i]);
                    usage(NULL);
                }
                continue;
            case 'n':          /* -name */
                if (++i >= argc)
                    usage("-name requires an argument");
                name = argv[i];
                continue;
            case 'r':
                switch (arg[2]) {
                case 'o':      /* -root */
                    use_root = True;
                    continue;
                case 'v':      /* -rv */
                    reverse = True;
                    continue;
                default:
                    goto unrecognized;
                }
                continue;
            case 's':          /* -s */
                attr.save_under = True;
                mask |= CWSaveUnder;
                continue;
            case 'e':          /* -event */
                if (++i >= argc)
                    usage("-event requires an argument");
                if (!parse_event_mask(argv[i], event_masks))
                    usage(NULL);
                event_mask_specified = True;
                continue;
            case '1':
                single_line = True;
                continue;
            case 'v':
                puts(PACKAGE_STRING);
                exit(0);
            default:
                goto unrecognized;
            }                   /* end switch on - */
        }
        else {
 unrecognized:
            fprintf(stderr, "%s: unrecognized argument '%s'\n",
                    ProgramName, arg);
            usage(NULL);
        }
    }                           /* end for over argc */

    /* if no -event options were specified, pretend all of them were */
    if (!event_mask_specified)
        parse_event_mask(NULL, event_masks);

    dpy = XOpenDisplay(displayname);
    if (!dpy) {
        fprintf(stderr, "%s:  unable to open display '%s'\n",
                ProgramName, XDisplayName(displayname));
        exit(1);
    }

    /* we're testing the default input method */
    modifiers = XSetLocaleModifiers("@im=none");
    if (modifiers == NULL) {
        fprintf(stderr, "%s:  XSetLocaleModifiers failed\n", ProgramName);
    }

    xim = XOpenIM(dpy, NULL, NULL, NULL);
    if (xim == NULL) {
        fprintf(stderr, "%s:  XOpenIM failed\n", ProgramName);
    }

    if (xim) {
        imvalret = XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL);
        if (imvalret != NULL || xim_styles == NULL) {
            fprintf(stderr, "%s:  input method doesn't support any styles\n",
                    ProgramName);
        }

        if (xim_styles) {
            xim_style = 0;
            for (i = 0; i < xim_styles->count_styles; i++) {
                if (xim_styles->supported_styles[i] ==
                    (XIMPreeditNothing | XIMStatusNothing)) {
                    xim_style = xim_styles->supported_styles[i];
                    break;
                }
            }

            if (xim_style == 0) {
                fprintf(stderr,
                        "%s: input method doesn't support the style we support\n",
                        ProgramName);
            }
            XFree(xim_styles);
        }
    }

    screen = DefaultScreen(dpy);

    attr.event_mask = event_masks[EVENT_MASK_INDEX_CORE];

    if (use_root)
        w = RootWindow(dpy, screen);

    if (w) {
        XGetWindowAttributes(dpy, w, &wattr);
        if (wattr.all_event_masks & ButtonPressMask)
            attr.event_mask &= ~ButtonPressMask;
        attr.event_mask &= ~SubstructureRedirectMask;
        XSelectInput(dpy, w, attr.event_mask);
    }
    else {
        set_sizehints(&hints, OUTER_WINDOW_MIN_WIDTH, OUTER_WINDOW_MIN_HEIGHT,
                      OUTER_WINDOW_DEF_WIDTH, OUTER_WINDOW_DEF_HEIGHT,
                      OUTER_WINDOW_DEF_X, OUTER_WINDOW_DEF_Y, geom);

        if (reverse) {
            back = BlackPixel(dpy, screen);
            fore = WhitePixel(dpy, screen);
        }
        else {
            back = WhitePixel(dpy, screen);
            fore = BlackPixel(dpy, screen);
        }

        attr.background_pixel = back;
        attr.border_pixel = fore;
        mask |= (CWBackPixel | CWBorderPixel | CWEventMask);

        w = XCreateWindow(dpy, RootWindow(dpy, screen), hints.x, hints.y,
                          hints.width, hints.height, borderwidth, 0,
                          InputOutput, (Visual *) CopyFromParent, mask, &attr);

        XSetStandardProperties(dpy, w, name, NULL, (Pixmap) 0,
                               argv, argc, &hints);

        subw = XCreateSimpleWindow(dpy, w, INNER_WINDOW_X, INNER_WINDOW_Y,
                                   INNER_WINDOW_WIDTH, INNER_WINDOW_HEIGHT,
                                   INNER_WINDOW_BORDER,
                                   attr.border_pixel, attr.background_pixel);

        wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
        wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, w, &wm_delete_window, 1);

        XMapWindow(dpy, subw);  /* map before w so that it appears */
        XMapWindow(dpy, w);

        printf("Outer window is 0x%lx, inner window is 0x%lx\n", w, subw);
    }

    if (xim && xim_style) {
        xic = XCreateIC(xim,
                        XNInputStyle, xim_style,
                        XNClientWindow, w, XNFocusWindow, w, NULL);

        if (xic == NULL) {
            fprintf(stderr, "XCreateIC failed\n");
        }
    }

    have_rr = XRRQueryExtension(dpy, &rr_event_base, &rr_error_base);
    if (have_rr) {
        int rr_major, rr_minor;

        if (XRRQueryVersion(dpy, &rr_major, &rr_minor)) {
            int rr_mask = event_masks[EVENT_MASK_INDEX_RANDR];

            if (rr_major == 1 && rr_minor <= 1) {
                rr_mask &= ~(RRCrtcChangeNotifyMask |
                             RROutputChangeNotifyMask |
                             RROutputPropertyNotifyMask);
            }

            XRRSelectInput(dpy, w, rr_mask);
        }
    }

    for (done = 0; !done;) {
        XEvent event;

        XNextEvent(dpy, &event);

        switch (event.type) {
        case KeyPress:
            prologue(&event, "KeyPress");
            do_KeyPress(&event);
            break;
        case KeyRelease:
            prologue(&event, "KeyRelease");
            do_KeyRelease(&event);
            break;
        case ButtonPress:
            prologue(&event, "ButtonPress");
            do_ButtonPress(&event);
            break;
        case ButtonRelease:
            prologue(&event, "ButtonRelease");
            do_ButtonRelease(&event);
            break;
        case MotionNotify:
            prologue(&event, "MotionNotify");
            do_MotionNotify(&event);
            break;
        case EnterNotify:
            prologue(&event, "EnterNotify");
            do_EnterNotify(&event);
            break;
        case LeaveNotify:
            prologue(&event, "LeaveNotify");
            do_LeaveNotify(&event);
            break;
        case FocusIn:
            prologue(&event, "FocusIn");
            do_FocusIn(&event);
            break;
        case FocusOut:
            prologue(&event, "FocusOut");
            do_FocusOut(&event);
            break;
        case KeymapNotify:
            prologue(&event, "KeymapNotify");
            do_KeymapNotify(&event);
            break;
        case Expose:
            prologue(&event, "Expose");
            do_Expose(&event);
            break;
        case GraphicsExpose:
            prologue(&event, "GraphicsExpose");
            do_GraphicsExpose(&event);
            break;
        case NoExpose:
            prologue(&event, "NoExpose");
            do_NoExpose(&event);
            break;
        case VisibilityNotify:
            prologue(&event, "VisibilityNotify");
            do_VisibilityNotify(&event);
            break;
        case CreateNotify:
            prologue(&event, "CreateNotify");
            do_CreateNotify(&event);
            break;
        case DestroyNotify:
            prologue(&event, "DestroyNotify");
            do_DestroyNotify(&event);
            break;
        case UnmapNotify:
            prologue(&event, "UnmapNotify");
            do_UnmapNotify(&event);
            break;
        case MapNotify:
            prologue(&event, "MapNotify");
            do_MapNotify(&event);
            break;
        case MapRequest:
            prologue(&event, "MapRequest");
            do_MapRequest(&event);
            break;
        case ReparentNotify:
            prologue(&event, "ReparentNotify");
            do_ReparentNotify(&event);
            break;
        case ConfigureNotify:
            prologue(&event, "ConfigureNotify");
            do_ConfigureNotify(&event);
            break;
        case ConfigureRequest:
            prologue(&event, "ConfigureRequest");
            do_ConfigureRequest(&event);
            break;
        case GravityNotify:
            prologue(&event, "GravityNotify");
            do_GravityNotify(&event);
            break;
        case ResizeRequest:
            prologue(&event, "ResizeRequest");
            do_ResizeRequest(&event);
            break;
        case CirculateNotify:
            prologue(&event, "CirculateNotify");
            do_CirculateNotify(&event);
            break;
        case CirculateRequest:
            prologue(&event, "CirculateRequest");
            do_CirculateRequest(&event);
            break;
        case PropertyNotify:
            prologue(&event, "PropertyNotify");
            do_PropertyNotify(&event);
            break;
        case SelectionClear:
            prologue(&event, "SelectionClear");
            do_SelectionClear(&event);
            break;
        case SelectionRequest:
            prologue(&event, "SelectionRequest");
            do_SelectionRequest(&event);
            break;
        case SelectionNotify:
            prologue(&event, "SelectionNotify");
            do_SelectionNotify(&event);
            break;
        case ColormapNotify:
            prologue(&event, "ColormapNotify");
            do_ColormapNotify(&event);
            break;
        case ClientMessage:
            prologue(&event, "ClientMessage");
            do_ClientMessage(&event);
            break;
        case MappingNotify:
            prologue(&event, "MappingNotify");
            do_MappingNotify(&event);
            break;
        default:
            if (have_rr) {
                if (event.type == rr_event_base + RRScreenChangeNotify) {
                    prologue(&event, "RRScreenChangeNotify");
                    do_RRScreenChangeNotify(&event);
                    break;
                }
                if (event.type == rr_event_base + RRNotify) {
                    do_RRNotify(&event);
                    break;
                }
            }
            output(NewLine, "Unknown event type %d", event.type);
            break;
        }
        if (single_line) {
            printf("\n");
        }
        fflush(stdout);
    }

    XCloseDisplay(dpy);
    return 0;
}
