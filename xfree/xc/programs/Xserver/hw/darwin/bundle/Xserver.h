//
//  Xserver.h
//
//  Created by Andreas Monitzer on January 6, 2001.
//
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/Xserver.h,v 1.8 2001/05/16 06:10:08 torrey Exp $ */

#import <Cocoa/Cocoa.h>

#include <drivers/event_status_driver.h>	// for NXEvent
#include <unistd.h>
#include <stdio.h>
#include <sys/syslimits.h>

@interface Xserver : NSObject {
    // server state
    NSLock *serverLock;
    NSTask *clientTask;
    NSPort *signalPort;
    BOOL serverVisible;
    BOOL appQuitting;
    UInt32 mouseState;

    // server event queue
    int eventWriteFD;

    // Aqua interface
    IBOutlet NSPanel *helpWindow;
    IBOutlet id startupHelpButton;
    IBOutlet NSPanel *switchWindow;
}

- (id)init;

- (BOOL)translateEvent:(NSEvent *)anEvent;

- (void)getNXMouse:(NXEvent*)ev;
+ (void)append:(NSString*)value toEnv:(NSString*)name;

- (void)run;
- (void)toggle;
- (void)show;
- (void)hide;
- (void)killServer;
- (void)readPasteboard;
- (void)writePasteboard;
- (void)clientTaskDone:(NSNotification *)aNotification;
- (void)sendNXEvent:(NXEvent*)ev;
- (void)sendShowHide:(BOOL)show;

// Aqua interface actions
- (IBAction)closeHelpAndShow:(id)sender;
- (IBAction)showAction:(id)sender;

// NSApplication delegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag;
- (void)applicationWillResignActive:(NSNotification *)aNotification;
- (void)applicationWillBecomeActive:(NSNotification *)aNotification;

// NSPort delegate
- (void)handlePortMessage:(NSPortMessage *)portMessage;

@end

