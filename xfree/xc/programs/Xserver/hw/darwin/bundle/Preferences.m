//
//  Preferences.m
//
//  This class keeps track of the user preferences.
//
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/Preferences.m,v 1.6 2001/05/09 07:16:19 torrey Exp $ */

#import "Preferences.h"
#import "quartzShared.h"

@implementation Preferences

+ (void)initialize {
    // Provide user defaults if needed
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
                    [NSNumber numberWithInt:0], @"Display",
                    @"YES", @"FakeButtons",
                    @"/System/Library/Keyboards/USA.keymapping", @"KeymappingFile",
                    @"NO", @"UseKeymappingFile",
                    @"Cmd-Opt-a", @"SwitchString",
                    @"YES", @"ShowStartupHelp",
                    [NSNumber numberWithInt:0], @"SwitchKeyCode",
                    [NSNumber numberWithInt:(NSCommandKeyMask | NSAlternateKeyMask)],
                    @"SwitchModifiers", @"NO", @"UseSystemBeep", 
                    @"NO", @"DockSwitch", nil];

    [super initialize];
    [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
}

// Initialize internal state info of switch key button
- (void)initSwitchKey {
    keyCode = [Preferences keyCode];
    modifiers = [Preferences modifiers];
    [switchString setString:[Preferences switchString]];
}

- (id)init {
    self = [super init];

    isGettingKeyCode=NO;
    switchString=[[NSMutableString alloc] init];
    [self initSwitchKey];

    return self;
}

// Set the window controls to the state in user defaults
- (void)resetWindow {
    [loadKeymapFileButton setIntValue:[Preferences useKeymapFile]];

    if ([Preferences keymapFile] == nil)
        [keymapFileField setStringValue:@" "];
    else
        [keymapFileField setStringValue:[Preferences keymapFile]];

    if ([Preferences switchString] == nil)
        [keyField setTitle:@"--"];
    else
        [keyField setTitle:[Preferences switchString]];

    [displayNumber setIntValue:[Preferences display]];
    [dockSwitchButton setIntValue:[Preferences dockSwitch]];
    [fakeButton setIntValue:[Preferences fakeButtons]];
    [startupHelpButton setIntValue:[Preferences startupHelp]];
    [systemBeepButton setIntValue:[Preferences systemBeep]];
}

- (void)awakeFromNib {
    [self resetWindow];
    [splashStartupHelpButton setIntValue:[Preferences startupHelp]];
}

// User cancelled the changes
- (IBAction)close:(id)sender
{
    [window orderOut:nil];
    [self resetWindow];  	// reset window controls
    [self initSwitchKey];	// reset switch key state
}

// Pick keymapping file
- (IBAction)pickFile:(id)sender
{
    int result;
    NSArray *fileTypes = [NSArray arrayWithObject:@"keymapping"];
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];

    [oPanel setAllowsMultipleSelection:NO];
    result = [oPanel runModalForDirectory:@"/System/Library/Keyboards" file:nil types:fileTypes];
    if (result == NSOKButton) {
        [keymapFileField setStringValue:[oPanel filename]];
    }
}

// User saved changes
- (IBAction)saveChanges:(id)sender
{
    [Preferences setKeyCode:keyCode];
    [Preferences setModifiers:modifiers];
    [Preferences setSwitchString:switchString];
    [Preferences setKeymapFile:[keymapFileField stringValue]];
    [Preferences setUseKeymapFile:[loadKeymapFileButton intValue]];
    [Preferences setDisplay:[displayNumber intValue]];
    [Preferences setDockSwitch:[dockSwitchButton intValue]];
    [Preferences setFakeButtons:[fakeButton intValue]];
    [Preferences setStartupHelp:[startupHelpButton intValue]];
    [Preferences setSystemBeep:[systemBeepButton intValue]];
    [Preferences saveToDisk];

    [window orderOut:nil];
}

- (IBAction)setKey:(id)sender
{
    [keyField setTitle:@"Press key"];
    isGettingKeyCode=YES;
    [switchString setString:@""];
}

- (BOOL)sendEvent:(NSEvent*)anEvent {
    if(isGettingKeyCode) {
        if([anEvent type]==NSKeyDown) //wait for keyup
            return YES;
        if([anEvent type]!=NSKeyUp)
            return NO;

        if([anEvent modifierFlags] & NSCommandKeyMask)
            [switchString appendString:@"Cmd-"];
        if([anEvent modifierFlags] & NSControlKeyMask)
            [switchString appendString:@"Ctrl-"];
        if([anEvent modifierFlags] & NSAlternateKeyMask)
            [switchString appendString:@"Opt-"];
        if([anEvent modifierFlags] & NSNumericPadKeyMask) // doesn't work
            [switchString appendString:@"Num-"];
        if([anEvent modifierFlags] & NSHelpKeyMask)
            [switchString appendString:@"Help-"];
        if([anEvent modifierFlags] & NSFunctionKeyMask) // powerbooks only
            [switchString appendString:@"Fn-"];
        
        [switchString appendString:[anEvent charactersIgnoringModifiers]];
        [keyField setTitle:switchString];
        
        keyCode = [anEvent keyCode];
        modifiers = [anEvent modifierFlags];
        isGettingKeyCode=NO;
        
        return YES;
    }
    return NO;
}

+ (void)setKeymapFile:(NSString*)newFile {
    [[NSUserDefaults standardUserDefaults] setObject:newFile forKey:@"KeymappingFile"];
}

+ (void)setUseKeymapFile:(BOOL)newUseKeymapFile {
    [[NSUserDefaults standardUserDefaults] setBool:newUseKeymapFile forKey:@"UseKeymappingFile"];
}

+ (void)setSwitchString:(NSString*)newString {
    [[NSUserDefaults standardUserDefaults] setObject:newString forKey:@"SwitchString"];
}

+ (void)setKeyCode:(int)newKeyCode {
    [[NSUserDefaults standardUserDefaults] setInteger:newKeyCode forKey:@"SwitchKeyCode"];
}

+ (void)setModifiers:(int)newModifiers {
    [[NSUserDefaults standardUserDefaults] setInteger:newModifiers forKey:@"SwitchModifiers"];
}

+ (void)setDisplay:(int)newDisplay {
    [[NSUserDefaults standardUserDefaults] setInteger:newDisplay forKey:@"Display"];
}

+ (void)setDockSwitch:(BOOL)newDockSwitch {
    [[NSUserDefaults standardUserDefaults] setBool:newDockSwitch forKey:@"DockSwitch"];
}

+ (void)setFakeButtons:(BOOL)newFakeButtons {
    [[NSUserDefaults standardUserDefaults] setBool:newFakeButtons forKey:@"FakeButtons"];
    // Update the setting used by the X server thread
    darwinFakeButtons = newFakeButtons;
}

+ (void)setStartupHelp:(BOOL)newStartupHelp {
    [[NSUserDefaults standardUserDefaults] setBool:newStartupHelp forKey:@"ShowStartupHelp"];
}

+ (void)setSystemBeep:(BOOL)newSystemBeep {
    [[NSUserDefaults standardUserDefaults] setBool:newSystemBeep forKey:@"UseSystemBeep"];
    // Update the setting used by the X server thread
    quartzUseSysBeep = newSystemBeep;
}

+ (void)saveToDisk {
    [[NSUserDefaults standardUserDefaults] synchronize];
}

+ (BOOL)useKeymapFile {
    return [[NSUserDefaults standardUserDefaults] boolForKey:@"UseKeymappingFile"];
}

+ (NSString*)keymapFile {
    return [[NSUserDefaults standardUserDefaults] stringForKey:@"KeymappingFile"];
}

+ (NSString*)switchString {
    return [[NSUserDefaults standardUserDefaults] stringForKey:@"SwitchString"];
}

+ (unsigned int)keyCode {
    return [[NSUserDefaults standardUserDefaults] integerForKey:@"SwitchKeyCode"];
}

+ (unsigned int)modifiers {
    return [[NSUserDefaults standardUserDefaults] integerForKey:@"SwitchModifiers"];
}

+ (int)display {
    return [[NSUserDefaults standardUserDefaults] integerForKey:@"Display"];
}

+ (BOOL)dockSwitch {
    return [[NSUserDefaults standardUserDefaults] boolForKey:@"DockSwitch"];
}

+ (BOOL)fakeButtons {
    return [[NSUserDefaults standardUserDefaults] boolForKey:@"FakeButtons"];
}

+ (BOOL)startupHelp {
    return [[NSUserDefaults standardUserDefaults] boolForKey:@"ShowStartupHelp"];
}

+ (BOOL)systemBeep {
    return [[NSUserDefaults standardUserDefaults] boolForKey:@"UseSystemBeep"];
}

@end
