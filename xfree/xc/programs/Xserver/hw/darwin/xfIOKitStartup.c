/**************************************************************
 *
 * Startup code for the IOKit Darwin X Server
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/xfIOKitStartup.c,v 1.3 2001/04/11 08:34:18 torrey Exp $ */

#include "mi.h"
#include "mipointer.h"
#include "scrnintstr.h"

/*
 * DarwinHandleGUI
 *  This function is called first from main().
 *  It does nothing for the IOKit X server.
 */
void DarwinHandleGUI(
    int         argc,
    char        *argv[],
    char        *envp[] )
{
}

// No Quartz support. All Quartz functions are no-ops.

BOOL QuartzAddScreen(ScreenPtr pScreen) {
    FatalError("QuartzAddScreen called without Quartz support.\n");
}

void QuartzOsVendorInit(void) {
    FatalError("QuartzOsVendorInit called without Quartz support.\n");
}

void QuartzGiveUp(void) {
    FatalError("QuartzGiveUp called without Quartz support.\n");
}

void QuartzHide(void) {
    FatalError("QuartzHide called without Quartz support.\n");
}

void QuartzShow(int x, int y) {
    FatalError("QuartzShow called without Quartz support.\n");
}

void QuartzReadPasteboard(void) {
    FatalError("QuartzReadPasteboard called without Quartz support.\n");
}

void QuartzWritePasteboard(void) {
    FatalError("QuartzWritePasteboard called without Quartz support.\n");
}

void QuartzBell(void) {
    FatalError("QuartzBell called without Quartz support.\n");
}
