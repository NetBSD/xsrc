/**************************************************************
 *
 * Startup code for the IOKit Darwin X Server
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/xfIOKitStartup.c,v 1.7 2001/12/22 05:28:34 torrey Exp $ */

#include "bundle/quartz.h"

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

// The IOKit X server does not accept any Quartz command line options.
int QuartzProcessArgument( int argc, char *argv[], int i )
{
    if (!strcmp( argv[i], "-fullscreen" ) ||
        !strcmp( argv[i], "-rootless" ) ||
        !strcmp( argv[i], "-quartz" ))
    {
        FatalError("Command line option %s is not available without Quartz "
                   "support.\nInstall the optional Xquartz.tgz tarball for "
                   "Quartz support.\n", argv[i]);
    }

    return 0;
}

// No Quartz support. All Quartz functions are no-ops.

Bool QuartzAddScreen(int index, ScreenPtr pScreen) {
    FatalError("QuartzAddScreen called without Quartz support.\n");
}

Bool QuartzSetupScreen(int index, ScreenPtr pScreen) {
    FatalError("QuartzInitCursor called without Quartz support.\n");
}

void QuartzInitOutput(int argc, char **argv) {
    FatalError("QuartzInitOutput called without Quartz support.\n");
}

void QuartzGiveUp(void) {
    return;	// no message, we are quitting anyway
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
