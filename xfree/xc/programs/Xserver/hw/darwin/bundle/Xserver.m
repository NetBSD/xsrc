//
//  Xserver.m
//
//  This class handles the interaction between the Cocoa front-end
//  and the Darwin X server thread.
//
//  Created by Andreas Monitzer on January 6, 2001.
//
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/Xserver.m,v 1.18 2001/05/16 06:10:08 torrey Exp $ */

#import "Xserver.h"
#import "Preferences.h"
#import "quartzShared.h"

// Macros to build the path name
#ifndef XBINDIR
#define XBINDIR /usr/X11R6/bin
#endif
#define STR(s) #s
#define XSTRPATH(s) STR(s)
#define XPATH(file) XSTRPATH(XBINDIR) "/" STR(file)

extern int argcGlobal;
extern char **argvGlobal;
extern char **envpGlobal;
extern int main(int argc, char *argv[], char *envp[]);
extern void HideMenuBar(void);
extern void ShowMenuBar(void);

static NSPortMessage *signalMessage;

@implementation Xserver

- (id)init {
    self=[super init];

    serverLock = [[NSLock alloc] init];
    clientTask = nil;
    serverVisible = NO;
    appQuitting = NO;
    mouseState = 0;
    eventWriteFD = quartzEventWriteFD;

    // set up a port to safely send messages to main thread from server thread
    signalPort = [[NSPort port] retain];
    signalMessage = [[NSPortMessage alloc] initWithSendPort:signalPort receivePort:signalPort components:nil];

    // set up receiving end
    [signalPort setDelegate:self];
    [[NSRunLoop currentRunLoop] addPort:signalPort forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addPort:signalPort forMode:NSModalPanelRunLoopMode];

    return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    // Quit if the X server is not running
    if ([serverLock tryLock])
        return NSTerminateNow;

    if ([clientTask isRunning] || !quartzStartClients) {
        int but;

        [self hide];
        but = NSRunAlertPanel(NSLocalizedString(@"Quit X server?",@""),
                              NSLocalizedString(@"Quitting the X server will terminate any running X Window programs.",@""),
                              NSLocalizedString(@"Quit",@""),
                              NSLocalizedString(@"Cancel",@""),
                              nil);

        switch (but) {
            case NSAlertDefaultReturn:		// quit
                break;
            case NSAlertAlternateReturn:	// cancel
                return NSTerminateCancel;
        }
    }

    appQuitting = YES;
    [self killServer];

    // Wait until the X server shuts down
    return NSTerminateLater;
}

// returns YES when event was handled
- (BOOL)translateEvent:(NSEvent *)anEvent {
    NXEvent ev;

    if(([anEvent type]==NSKeyDown) && (![anEvent isARepeat]) &&
       ([anEvent keyCode]==[Preferences keyCode]) &&
       ([anEvent modifierFlags]==[Preferences modifiers])) {
        [self toggle];
        return YES;
    }

    if(!serverVisible)
        return NO;

    [self getNXMouse:&ev];
    ev.type=[anEvent type];
    ev.flags=[anEvent modifierFlags];
    switch(ev.type) {
        case NSLeftMouseDown:
        case NSLeftMouseUp:
        case NSMouseMoved:
            break;
        case NSLeftMouseDragged:
        case NSRightMouseDragged:
        case 27:        // undocumented high button MouseDragged event
            ev.type=NSMouseMoved;
            break;
        case NSSystemDefined:
            if (![anEvent subtype]==7)
                return NO; // we only use multibutton mouse events
            if ([anEvent data1] & 1)
                return NO; // skip mouse button 1 events
            if (mouseState==[anEvent data2])
                return NO; // ignore double events
            ev.data.compound.subType=[anEvent subtype];
            ev.data.compound.misc.L[0]=[anEvent data1];
            ev.data.compound.misc.L[1]=mouseState=[anEvent data2];
            break;
        case NSScrollWheel:
            ev.data.scrollWheel.deltaAxis1=[anEvent deltaY];
            break;
        case NSKeyDown:
        case NSKeyUp:
            ev.data.key.keyCode = [anEvent keyCode];
            ev.data.key.repeat = [anEvent isARepeat];
            break;
        case NSFlagsChanged:
            ev.data.key.keyCode = [anEvent keyCode];
            break;
        case 25:        // undocumented MouseDown
        case 26:        // undocumented MouseUp
            // Hide these from AppKit to avoid its log messages
            return YES;
        default:
            return NO;
    }

    [self sendNXEvent:&ev];

    return YES;
}

- (void)getNXMouse:(NXEvent*)ev {
    NSPoint pt=[NSEvent mouseLocation];
    ev->location.x=(int)(pt.x);
    ev->location.y=[[NSScreen mainScreen] frame].size.height-(int)(pt.y); // invert mouse
}

// Append a string to the given enviroment variable
+ (void)append:(NSString*)value toEnv:(NSString*)name {
    setenv([name cString],
        [[[NSString stringWithCString:getenv([name cString])]
            stringByAppendingString:value] cString],1);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Start the X server thread
    [NSThread detachNewThreadSelector:@selector(run) toTarget:self withObject:nil];

    // If we are going to display a splash screen, hide the X11 screen immediately
    if ([Preferences startupHelp])
        [self sendShowHide:NO];

    // Start the X clients if started from GUI
    if (quartzStartClients) {
        char *home;
        char xinitrcbuf[PATH_MAX];
        NSString *path = [NSString stringWithCString:XPATH(xinit)];
        NSString *server = [NSString stringWithCString:XPATH(XDarwinStartup)];
        NSString *client, *displayName;
        BOOL hasClient = YES;
        NSArray *args;

        // Register to receive notification when the client task finishes
        [[NSNotificationCenter defaultCenter] addObserver:self 
                selector:@selector(clientTaskDone:) 
                name:NSTaskDidTerminateNotification 
                object:nil];

        // Change to user's home directory (so xterms etc. start there)
        home = getenv("HOME");
        if (home)
            chdir(home);
        else
            home = "";

        // Add X binary directory to path
        [Xserver append:@":" toEnv:@"PATH"];
        [Xserver append:@XSTRPATH(XBINDIR) toEnv:@"PATH"];

        displayName = [NSString localizedStringWithFormat:@":%d",
                                [Preferences display]];

        // Find the client init file to use
        snprintf(xinitrcbuf, PATH_MAX, "%s/.xinitrc", home);
        if (access(xinitrcbuf, F_OK)) {
            snprintf(xinitrcbuf, PATH_MAX, XSTRPATH(XINITDIR) "/xinitrc");
            if (access(xinitrcbuf, F_OK)) {
                hasClient = NO;
            }
        }
        if (hasClient) {
            client = [NSString stringWithCString:xinitrcbuf];
            args = [NSArray arrayWithObjects:client, @"--", server,
                            displayName, @"-idle", nil];
        } else {
            args = [NSArray arrayWithObjects:@"--", server, displayName,
                            @"-idle", nil];
        }

        // Launch a new task to run start X clients
        clientTask = [NSTask launchedTaskWithLaunchPath:path arguments:args];
    }

    // Make sure the menu bar gets drawn
    [NSApp setWindowsNeedUpdate:YES];

    // Show the X switch window if not using dock icon switching
    if (![Preferences dockSwitch])
        [switchWindow orderFront:nil];

    // Display the help splash screen or show the X server
    if ([Preferences startupHelp]) {
        [helpWindow makeKeyAndOrderFront:nil];
    } else {
        ShowMenuBar();
        [self closeHelpAndShow:nil];
    }
}

// Run the X server thread
- (void)run {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    [serverLock lock];
    main(argcGlobal, argvGlobal, envpGlobal);
    serverVisible = NO;
    [serverLock unlock];
    [pool release];
    QuartzMessageMainThread(kQuartzServerDied);
}

// Close the help splash screen and show the X server
- (IBAction)closeHelpAndShow:(id)sender {
    int helpVal;

    helpVal = [startupHelpButton intValue];
    [Preferences setStartupHelp:helpVal];
    [Preferences saveToDisk];

    [helpWindow close];

    serverVisible = YES;
    [self sendShowHide:YES];
    [NSApp activateIgnoringOtherApps:YES];
}

// Show the X server when sent message from GUI
- (IBAction)showAction:(id)sender {
    [self sendShowHide:YES];
}

// Show or hide the X server
- (void)toggle {
    if (serverVisible)
        [self hide];
    else
        [self show];
}

// Show the X server on screen
- (void)show {
    if (!serverVisible) {
        [self sendShowHide:YES];
    }
}

// Hide the X server from the screen
- (void)hide {
    if (serverVisible) {
        [self sendShowHide:NO];
    }
}

// Kill the Xserver thread
- (void)killServer {
    NXEvent ev;

    if (serverVisible)
        [self hide];

    ev.type = NX_APPDEFINED;
    ev.data.compound.subType = kXDarwinQuit;
    [self sendNXEvent:&ev];
}

// Tell the X server to show or hide itself.
// This ignores the current X server visible state.
- (void)sendShowHide:(BOOL)show {
    NXEvent ev;

    [self getNXMouse:&ev];
    ev.type = NX_APPDEFINED;

    if (show) {
        QuartzCapture();
        HideMenuBar();
        ev.data.compound.subType = kXDarwinShow;
        [self sendNXEvent:&ev];

        // inform the X server of the current modifier state
        ev.flags = [[NSApp currentEvent] modifierFlags];
        ev.data.compound.subType = kXDarwinUpdateModifiers;
        [self sendNXEvent:&ev];

        // put the pasteboard into the X cut buffer
        [self readPasteboard];
    } else {
        // put the X cut buffer on the pasteboard
        [self writePasteboard];

        ev.data.compound.subType = kXDarwinHide;
        [self sendNXEvent:&ev];
        ShowMenuBar();
    }

    serverVisible = show;
}

// Tell the X server to read from the pasteboard into the X cut buffer
- (void)readPasteboard 
{
    NXEvent ev;

    ev.type = NX_APPDEFINED;
    ev.data.compound.subType = kXDarwinReadPasteboard;
    [self sendNXEvent:&ev];
}

// Tell the X server to write the X cut buffer into the pasteboard
- (void)writePasteboard 
{
    NXEvent ev;

    ev.type = NX_APPDEFINED;
    ev.data.compound.subType = kXDarwinWritePasteboard;
    [self sendNXEvent:&ev];
}

- (void)sendNXEvent:(NXEvent*)ev {
    if (write(eventWriteFD, ev, sizeof(*ev)) == sizeof(*ev))
        return;
    NSLog(@"Bad write to event pipe.");
    // FIXME: handle bad writes better?
}

// Handle messages from the X server thread
- (void)handlePortMessage:(NSPortMessage *)portMessage {
    unsigned msg = [portMessage msgid];

    switch(msg) {
        case kQuartzServerHidden:
            // FIXME: This hack is necessary (but not completely effective)
            // since Mac OS X 10.0.2
            [NSCursor unhide];
            break;
        case kQuartzServerDied:
            if (appQuitting) {
                // If we quit before the clients start, they may sit and wait
                // for the X server to start. Kill them instead.
                if ([clientTask isRunning])
                    [clientTask terminate];
                [NSApp replyToApplicationShouldTerminate:YES];
            } else {
                [NSApp terminate:nil];	// quit if we aren't already
            }
            break;
        default:
            NSLog(@"Unknown message from server thread.");
    }
}

// Quit the X server when the X client task finishes
- (void)clientTaskDone:(NSNotification *)aNotification {
    // Make sure it was the client task that finished
    if (![clientTask isRunning]) {
        int status = [[aNotification object] terminationStatus];

        if (status != 0)
            NSLog(@"X client task terminated abnormally.");

        if (!appQuitting)
            [NSApp terminate:nil];	// quit if we aren't already
    }
}

// Called when the user clicks the application icon, but not when Cmd-Tab is used
- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag {
    if ([Preferences dockSwitch]) {
        [self show];
    }
    return NO;
}

- (void)applicationWillResignActive:(NSNotification *)aNotification {
    [self hide];
}

- (void)applicationWillBecomeActive:(NSNotification *)aNotification {
    [self readPasteboard];
}

@end

// Send a message to the main thread, which calls handlePortMessage in
// response. Must only be called from the X server thread because
// NSPort is not thread safe.
void QuartzMessageMainThread(unsigned msg) {
    [signalMessage setMsgid:msg];
    [signalMessage sendBeforeDate:[NSDate distantPast]];
}
