/**************************************************************
 *
 * Quartz-specific support for the Darwin X Server
 * that requires Cocoa and Objective-C.
 *
 * This file is separate from the parts of Quartz support
 * that use X include files to avoid symbol collisions.
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzCocoa.m,v 1.5 2001/04/28 20:42:19 torrey Exp $ */

#include <Cocoa/Cocoa.h>

#import "Preferences.h"
#include "quartzShared.h"

extern void FatalError(const char *, ...);
extern char *display;

// Read the user preferences from the Cocoa front end
void QuartzReadPreferences(void)
{
    char *fileString;

    darwinFakeButtons = [Preferences fakeButtons];
    quartzUseSysBeep = [Preferences systemBeep];

    if ([Preferences useKeymapFile]) {
        fileString = (char *) [[Preferences keymapFile] lossyCString];
        darwinKeymapFile = (char *) malloc(strlen(fileString)+1);
        if (! darwinKeymapFile)
            FatalError("malloc failed in QuartzReadPreferences()!\n");
        strcpy(darwinKeymapFile, fileString);
    }

    display = (char *) malloc(8);
    if (! display)
        FatalError("malloc failed in QuartzReadPreferences()!\n");
    snprintf(display, 8, "%i", [Preferences display]);
}

// Write text to the Mac OS X pasteboard.
void QuartzWriteCocoaPasteboard(char *text)
{
    NSPasteboard *pasteboard;
    NSArray *pasteboardTypes;
    NSString *string;

    if (! text) return;
    pasteboard = [NSPasteboard generalPasteboard];
    if (! pasteboard) return;
    string = [NSString stringWithCString:text];
    if (! string) return;
    pasteboardTypes = [NSArray arrayWithObject:NSStringPboardType];

    // nil owner because we don't provide type translations
    [pasteboard declareTypes:pasteboardTypes owner:nil];
    [pasteboard setString:string forType:NSStringPboardType];
}

// Read text from the Mac OS X pasteboard and return it as a heap string.
// The caller must free the string.
char *QuartzReadCocoaPasteboard(void)
{
    NSPasteboard *pasteboard;
    NSArray *pasteboardTypes;
    NSString *existingType;
    char *text = NULL;
    
    pasteboardTypes = [NSArray arrayWithObject:NSStringPboardType];    
    pasteboard = [NSPasteboard generalPasteboard];
    if (! pasteboard) return NULL;

    existingType = [pasteboard availableTypeFromArray:pasteboardTypes];
    if (existingType) {
        NSString *string = [pasteboard stringForType:existingType];
        char *buffer;

        if (! string) return NULL;
        buffer = (char *) [string lossyCString];
        text = (char *) malloc(strlen(buffer)+1);
        if (text)
            strcpy(text, buffer);
    }

    return text;
}
