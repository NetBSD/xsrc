/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/fakeBoxRec.h,v 1.1 2001/07/01 02:13:41 torrey Exp $ */

#ifndef FAKEBOXREC_H
#define FAKEBOXREC_H

// This struct is byte-compatible with X11's BoxRec, for use in
// code that can't include X headers.
typedef struct _fakeBox {
    short x1;
    short y1;
    short x2;
    short y2;
} fakeBoxRec;

#endif
