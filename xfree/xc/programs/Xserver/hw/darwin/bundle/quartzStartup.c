/**************************************************************
 *
 * Startup code for the Quartz Darwin X Server
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzStartup.c,v 1.7 2001/09/23 04:04:49 torrey Exp $ */

#include <fcntl.h>
#include "quartzCommon.h"
#include "darwin.h"
#include "opaque.h"

int NSApplicationMain(int argc, char *argv[]);

char **envpGlobal;      // argcGlobal and argvGlobal
                        // are from dix/globals.c

/*
 * DarwinHandleGUI
 *  This function is called first from main(). The first time
 *  it is called we start the Mac OS X front end. The front end
 *  will call main() again from another thread to run the X
 *  server. On the second call this function loads the user
 *  preferences set by the Mac OS X front end.
 */
void DarwinHandleGUI(
    int         argc,
    char        *argv[],
    char        *envp[] )
{
    static Bool been_here = FALSE;
    int         main_exit, i;
    int         fd[2];

    if (been_here) {
        QuartzReadPreferences();
        return;
    }
    been_here = TRUE;

    // Make a pipe to pass events
    assert( pipe(fd) == 0 );
    darwinEventFD = fd[0];
    quartzEventWriteFD = fd[1];
    fcntl(darwinEventFD, F_SETFL, O_NONBLOCK);

    // Store command line arguments to pass back to main()
    argcGlobal = argc;
    argvGlobal = argv;
    envpGlobal = envp;

    // Determine if we need to start X clients
    // and what display mode to use
    quartzStartClients = 1;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-nostartx")) {
            quartzStartClients = 0;    
        } else if (!strcmp( argv[i], "-fullscreen")) {
            quartzRootless = 0;
        } else if (!strcmp( argv[i], "-rootless")) {
            quartzRootless = 1;
        }
    }

    quartz = TRUE;
    main_exit = NSApplicationMain(argc, argv);
    exit(main_exit);
}

int QuartzProcessArgument( int argc, char *argv[], int i )
{
    // fullscreen: CoreGraphics full-screen mode
    // rootless: Cocoa rootless mode
    // quartz: Default, either fullscreen or rootless

    if ( !strcmp( argv[i], "-fullscreen" ) ) {
        ErrorF( "Running full screen in parallel with Mac OS X Quartz window server.\n" );
#ifdef QUARTZ_SAFETY_DELAY
        ErrorF( "Quitting in %d seconds if no controller is found.\n",
                QUARTZ_SAFETY_DELAY );
#endif
        return 1;
    }

    if ( !strcmp( argv[i], "-rootless" ) ) {
        ErrorF( "Running rootless inside Mac OS X window server.\n" );
#ifdef QUARTZ_SAFETY_DELAY
        ErrorF( "Quitting in %d seconds if no controller is found.\n",
                QUARTZ_SAFETY_DELAY );
#endif
        return 1;
     }

    if ( !strcmp( argv[i], "-quartz" ) ) {
        ErrorF( "Running in parallel with Mac OS X Quartz window server.\n" );
#ifdef QUARTZ_SAFETY_DELAY
        ErrorF( "Quitting in %d seconds if no controller is found.\n",
                QUARTZ_SAFETY_DELAY );
#endif
        return 1;
    }

    // The Mac OS X front end uses this argument, which we just ignore here.
    if ( !strcmp( argv[i], "-nostartx" ) ) {
        return 1;
    }

    // This command line arg is passed when launched from the Aqua GUI.
    if ( !strncmp( argv[i], "-psn_", 5 ) ) {
        return 1;
    }

    return 0;
}