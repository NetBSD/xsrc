/*
 * NSWindow subclass for Mac OS X rootless X server
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/XWindow.h,v 1.2 2001/09/17 03:08:40 torrey Exp $ */

#import <Cocoa/Cocoa.h>
#import "XView.h"

#include "fakeBoxRec.h"

@interface XWindow : NSWindow
{
    XView *mView;
}

- (id)initWithContentRect:(NSRect)aRect
                   isRoot:(BOOL)isRoot;
- (void)dealloc;

- (char *)bits;
- (void)getBits:(char **)bits
       rowBytes:(int *)rowBytes
          depth:(int *)depth
   bitsPerPixel:(int *)bpp;

- (void)refreshRects:(fakeBoxRec *)rectList
               count:(int)count;

- (void)orderWindow:(NSWindowOrderingMode)place
         relativeTo:(int)otherWindowNumber;

- (void)sendEvent:(NSEvent *)anEvent;
- (BOOL)canBecomeMainWindow;
- (BOOL)canBecomeKeyWindow;

@end
