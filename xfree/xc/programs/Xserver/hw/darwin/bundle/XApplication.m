//
//  XApplication.m
//  Xmaster project
//
//  Created by Andreas Monitzer on January 6, 2001.
//

#import "XApplication.h"


@implementation XApplication

- (void)sendEvent:(NSEvent*)anEvent {
    if(![xserver translateEvent:anEvent]) {
        if(![preferences sendEvent:anEvent])
            [super sendEvent:anEvent];
    }
}

@end
