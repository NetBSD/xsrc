/* $Xorg: dsimple.c,v 1.4 2001/02/09 02:05:54 xorgcvs Exp $ */
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

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <regex.h>
/*
 * Other_stuff.h: Definitions of routines in other_stuff.
 *
 * Written by Mark Lillibridge.   Last updated 7/1/87
 */

#include "dsimple.h"

/*
 * Just_display: A group of routines designed to make the writing of simple
 *               X11 applications which open a display but do not open
 *               any windows much faster and easier.  Unless a routine says
 *               otherwise, it may be assumed to require program_name, dpy,
 *               and screen already defined on entry.
 *
 * Written by Mark Lillibridge.   Last updated 7/1/87
 */


/* This stuff is defined in the calling program by just_display.h */
const char *program_name = "unknown_program";
Display *dpy;
int screen;

/*
 * Get_Display_Name (argc, argv) Look for -display, -d, or host:dpy (obselete)
 * If found, remove it from command line.  Don't go past a lone -.
 */
static char *
Get_Display_Name (int *pargc, char **argv)
{
    int argc = *pargc;
    char **pargv = argv + 1;
    char *displayname = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (!strcmp (arg, "-display") || !strcmp (arg, "-d")) {
            if (++i >= argc)
                Usage ();

            displayname = argv[i];
            *pargc -= 2;
            continue;
        }
        if (!strcmp (arg, "-")) {
            while (i <argc)
                *pargv++ = argv[i++];
            break;
        }
        *pargv++ = arg;
    }

    *pargv = NULL;
    return (displayname);
}


/*
 * Open_Display: Routine to open a display with correct error handling.
 *               Does not require dpy or screen defined on entry.
 */
static Display *
Open_Display (char *display_name)
{
    Display *d;

    d = XOpenDisplay (display_name);
    if (d == NULL) {
        fprintf (stderr, "%s:  unable to open display '%s'\n",
                 program_name, XDisplayName (display_name));
        Usage ();
    }

    return (d);
}


/*
 * Setup_Display_And_Screen: This routine opens up the correct display (i.e.,
 *                           it calls Get_Display_Name) and then stores a
 *                           pointer to it in dpy.  The default screen
 *                           for this display is then stored in screen.
 *                           Does not require dpy or screen defined.
 */
void
Setup_Display_And_Screen (int *argc, char **argv)
{
    dpy = Open_Display (Get_Display_Name (argc, argv));
    screen = DefaultScreen (dpy);
}

/*
 * Other_stuff: A group of routines which do common X11 tasks.
 *
 * Written by Mark Lillibridge.   Last updated 7/1/87
 */

/*
 * Standard fatal error routine - call like printf but maximum of 7 arguments.
 * Does not require dpy or screen defined.
 */
void
Fatal_Error (const char *msg, ...)
{
    va_list args;
    fflush (stdout);
    fflush (stderr);
    fprintf (stderr, "%s: error: ", program_name);
    va_start (args, msg);
    vfprintf (stderr, msg, args);
    va_end (args);
    fprintf (stderr, "\n");
    exit (1);
}

/*
 * Routine to let user select a window using the mouse
 */

Window
Select_Window (Display *disp)
{
    int status;
    Cursor cursor;
    XEvent event;
    Window target_win = None, root = RootWindow (disp, screen);
    int buttons = 0;
      
    /* Make the target cursor */
    cursor = XCreateFontCursor (disp, XC_crosshair);

    /* Grab the pointer using target cursor, letting it room all over */
    status = XGrabPointer (disp, root, False,
                           ButtonPressMask|ButtonReleaseMask, GrabModeSync,
                           GrabModeAsync, root, cursor, CurrentTime);
    if (status != GrabSuccess)
        Fatal_Error ("Can't grab the mouse.");

    /* Let the user select a window... */
    while ((target_win == None) || (buttons != 0)) {
        /* allow one more event */
        XAllowEvents (disp, SyncPointer, CurrentTime);
        XWindowEvent (disp, root, ButtonPressMask | ButtonReleaseMask, &event);
        switch (event.type) {
        case ButtonPress:
            if (target_win == None) {
                target_win = event.xbutton.subwindow; /* window selected */
                if (target_win == None)
                    target_win = root;
            }
            buttons++;
            break;
        case ButtonRelease:
            if (buttons > 0) /* there may have been some down before we started */
                buttons--;
            break;
        }
    }

    XUngrabPointer (disp, CurrentTime); /* Done with pointer */

    return (target_win);
}

/*
 * Routine that returns the window currently under the cursor
 * Added by Daniel Forchheimer.   Last updated 19/12/04
 */

Window
Get_Window_Under_Cursor (Display *disp)
{
    int status;
    Cursor cursor;
    //XEvent event;
    Window target_win = None, root = RootWindow (disp, screen);
    //int buttons = 0;
    Window tmp;
    int rx, ry, cx, cy;
    unsigned int mask;

    /* Make the target cursor */
    cursor = XCreateFontCursor (disp, XC_crosshair);

    /* Grab the pointer using target cursor, letting it room all over */
    status = XGrabPointer (disp, root, False,
                           ButtonPressMask|ButtonReleaseMask, GrabModeSync,
                           GrabModeAsync, root, cursor, CurrentTime);
    if (status != GrabSuccess)
        Fatal_Error ("Can't grab the mouse.");

    /* get the window under the cursor */
    XQueryPointer (disp, root, &tmp, &target_win, &rx, &ry,
                   &cx, &cy, &mask);

    XUngrabPointer (disp, CurrentTime);      /* Done with pointer */

    return (target_win);
}

/*
 * Window_With_Name: routine to locate a window with a given name on a display.
 *                   If no window with the given name is found, 0 is returned.
 *                   If more than one window has the given name, the first
 *                   one found will be returned.  Only top and its subwindows
 *                   are looked at.  Normally, top should be the RootWindow.
 */
Window
Window_With_Name (Display *disp, Window top, char *name)
{
    Window *children, dummy;
    unsigned int nchildren, i;
    Window w = 0;
    char *window_name;

    if (XFetchName (disp, top, &window_name) && !strcmp (window_name, name))
        return (top);

    if (!XQueryTree (disp, top, &dummy, &dummy, &children, &nchildren))
        return (0);

    for (i = 0; i < nchildren; i++) {
        w = Window_With_Name (disp, children[i], name);
        if (w)
            break;
    }
    if (children)
        XFree ((char *) children);
    return (w);
}

/*
 * Window_With_Name_Regex: Same as above but use regular expressions
 *                         to match a window name. Only returns the first
 *                         result.
 * Window_With_Name_Regex_Recurse: Takes regex_t struct as argument instead of char*
 * Written by Daniel Forchheimer 2005
 * */
static Window
Window_With_Name_Regex_Recurse (Display *disp, Window top,
                                regex_t *reg_name)
{
    Window *children, dummy;
    unsigned int nchildren, i;
    Window w = 0;
    char *window_name;

    if (XFetchName (disp, top, &window_name) &&
        !regexec (reg_name, window_name, 0, NULL, 0))
        return (top);

    if (!XQueryTree (disp, top, &dummy, &dummy, &children, &nchildren))
        return (0);

    for (i = 0; i < nchildren; i++) {
        w = Window_With_Name_Regex_Recurse (disp, children[i], reg_name);
        if (w)
            break;
    }
    if (children)
        XFree ((char *) children);
    return (w);
}

/* prepare the reg-exp for use with above function */
Window
Window_With_Name_Regex (Display *disp, Window top, char *name)
{
    int err_no = 0;
    regex_t *regexp_name;
    Window target_win;
    regexp_name = (regex_t *) malloc (sizeof (regex_t));
    if ((err_no = regcomp (regexp_name, name, 0)) != 0) {
        size_t length;
        char *buffer;
        length = regerror (err_no, regexp_name, NULL, 0);
        buffer = malloc (length);
        regerror (err_no, regexp_name, buffer, length);
        fprintf (stderr, "%s\n", buffer);
        free (buffer);
        regfree (regexp_name);
        exit (1);
    }
    target_win = Window_With_Name_Regex_Recurse (disp,
                                                 RootWindow (disp, screen),
                                                 regexp_name);

    regfree (regexp_name);
    free (regexp_name);
    return target_win;
}
