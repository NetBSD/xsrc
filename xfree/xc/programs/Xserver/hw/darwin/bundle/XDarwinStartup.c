/**************************************************************
 *
 * Startup program for Darwin X servers
 *
 * This program selects the appropriate X server to launch:
 *  XDarwin         IOKit X server (default)
 *  XDarwinQuartz   A soft link to the Quartz X server
 *                  executable (-quartz option)
 *
 * If told to idle, the program will simply pause and not
 * launch any X server. This is to support startx being
 * run by XDarwin.app.
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/XDarwinStartup.c,v 1.3 2001/04/16 06:51:48 torrey Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslimits.h>

extern int errno;

// Macros to build the path name
#ifndef XBINDIR
#define XBINDIR /usr/X11R6/bin
#endif
#define STR(s) #s
#define XSTRPATH(s) STR(s) "/"
#define XPATH(file) XSTRPATH(XBINDIR) STR(file)

int main(
    int         argc,
    char        *argv[] )
{
    int         i, j;
    char        **newargv;

    // Check if we are going to run in Quartz mode or idle
    // to support startx from the Quartz server. The last
    // parameter in the list is the one used.
    for (i = argc-1; i; i--) {
        if (!strcmp(argv[i], "-idle")) {
            pause();
            return 0;

        } else if (!strcmp(argv[i], "-quartz")) {
            char quartzPath[PATH_MAX+1];
            int pathLength;

            // Find the path to the Quartz executable
            pathLength = readlink(XPATH(XDarwinQuartz), quartzPath, PATH_MAX);
            if (!pathLength) {
                fprintf(stderr, "The symbolic link " XPATH(XDarwinQuartz)
                        " is not valid.\n");
                return errno;
            }
            quartzPath[pathLength] = '\0';

            // Build the new argument list
            newargv = (char **) malloc((argc+2) * sizeof(char *));
            for (j = argc; j; j--)
                newargv[j] = argv[j];
            newargv[0] = quartzPath;
            newargv[argc] = "-nostartx";
            newargv[argc+1] = NULL;

            execv(newargv[0], newargv);
            fprintf(stderr, "Could not start XDarwin Quartz X server.\n");
            return errno;
        }
    }

    // Build the new argument list
    newargv = (char **) malloc((argc+1) * sizeof(char *));
    for (j = argc; j; j--)
        newargv[j] = argv[j];
    newargv[0] = XPATH(XDarwin);
    newargv[argc] = NULL;

    // Launch the IOKit X server
    execv(newargv[0], newargv);
    fprintf(stderr, "Could not start XDarwin IOKit X server.\n");
    return errno;
}
