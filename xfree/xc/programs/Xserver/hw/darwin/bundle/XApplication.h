//
//  NSXApplication.h
//  Xmaster project
//
//  Created by Andreas Monitzer on January 6, 2001.
//

#import <Cocoa/Cocoa.h>

#import "Xserver.h"
#import "Preferences.h"

@interface XApplication : NSApplication {
    IBOutlet Xserver *xserver;
    IBOutlet Preferences *preferences;
}

- (void)sendEvent:(NSEvent *)anEvent;

@end
