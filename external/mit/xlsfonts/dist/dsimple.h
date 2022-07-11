/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

/*
 * dsimple.h:      This file contains the definitions needed to use the
 *                 functions in dsimple.c.  It also declares the global
 *                 variables dpy, screen, and program_name which are needed to
 *                 use dsimple.c.
 *
 * Written by Mark Lillibridge.   Last updated 7/1/87
 *
 * Send bugs, etc. to chariot@athena.mit.edu.
 */

#include <X11/Xfuncproto.h>

    /* Simple helper macros */
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif /* MAX */
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */

    /* Global variables used by routines in just_display.c */

extern const char *program_name;        /* Name of this program */
extern Display *dpy;                    /* The current display */
extern int screen;                      /* The current screen */

#define INIT_NAME program_name=argv[0]  /* use this in main to setup
                                           program_name */

/* Declarations for functions in dsimple.c */

void Setup_Display_And_Screen(int *, char **);
void Close_Display(void);
void usage(const char *errmsg) _X_NORETURN;

#define X_USAGE "[host:display]"        /* X arguments handled by
                                           Get_Display_Name */

void Fatal_Error(const char *, ...) _X_NORETURN _X_ATTRIBUTE_PRINTF(1, 2);
