/**************************************************************
 *
 * Startup code for the Quartz Darwin X Server
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzStartup.c,v 1.3 2001/04/05 06:08:46 torrey Exp $ */

#include <fcntl.h>
#include "opaque.h"
#include "../darwin.h"
#include "quartzShared.h"

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
    quartzStartClients = 1;
    for (i = argc-1; i; i--) {
        if (!strcmp(argv[i], "-nostartx")) {
            quartzStartClients = 0;
        }
    }

    quartz = TRUE;
    main_exit = NSApplicationMain(argc, argv);
    exit(main_exit);
}
