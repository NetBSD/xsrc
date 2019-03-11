/* Copyright © 2003-2004 Matthew Hawn
 * Copyright © 2003-2004 Andreas Kohn
 * Copyright © 2003-2004 Roman Divacky
 * Copyright © 2003-2004 Keith Packard
 * Copyright © 2005-2007 Daniel Forchheimer
 * Copyright © 2011-2012 Arnaud Fontaine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define VERSIONSTR "6"

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "dsimple.h"
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>

Window target_win;

/* needed by dsimple.c */

void
Usage (void)
{
    fputs ("usage: transset [-options ...] [opacity]\n"
           "options:\n"
           "    -h, --help           display this message\n"
           "    -t, --toggle         force toggle of opacity\n"
           "    -c, --click          select by clicking on window  (default)\n"
           "    -p, --point          select the window currently under the cursor\n"
           "    -a, --actual         select the actual window\n"
           "    -n, --name NAME      select by name, NAME is matched as regular expression\n"
           "    --no-regex           don't use regular expression for matching name\n"
           "    -i, --id             select by window id\n"
           "        --inc            increase by the given opacity\n"
           "        --dec            decrease by given opacity\n"
           "    -m, --min OPACITY    minimum possible opacity  (default = 0)\n"
           "    -x, --max OPACITY    maximum possible opacity  (default = 1)\n"
           "    -v, --verbose        print some debug info\n"
           "    -V, --version        print version number\n",
        stderr);

    exit (1);
}

#define OPAQUE	0xffffffff
#define OPACITY	"_NET_WM_WINDOW_OPACITY"

/* returns the highest parent of child that is not the root-window */
static Window
Get_Top_Window (Display *disp, Window child) {
    Window parent;
    Window root;
    Window *child_list;
    unsigned int num_children;

    if (!XQueryTree (disp, child, &root, &parent, &child_list,
                     &num_children))
        Fatal_Error ("Can't query window tree.");

    XFree ((void *) child_list);
    if (parent == root)
        return child;

    while (parent != root) {
        child = parent;
        if (!XQueryTree (disp, child, &root, &parent, &child_list,
                         &num_children))
            Fatal_Error ("Can't query window tree.");
        XFree ((void *) child_list);
    }
    return child;
}

static Window
Get_Actual_Window (Display *disp)
{
    int i;
    Window w;

    XGetInputFocus (disp, &w, &i);
    return Get_Top_Window (disp, w);
}

typedef enum {
    SELECT_METHOD_CLICK = 0,
    SELECT_METHOD_WINDOW_UNDER_CURSOR = 1,
    SELECT_METHOD_WINDOW_ID = 2,
    SELECT_METHOD_WINDOW_NAME = 3,
    SELECT_METHOD_FOCUSED_WINDOW = 4
} select_method_t;

int
main (int argc, char **argv)
{
    Bool gotd = False;
    double d = 0.75;
    unsigned int opacity;
    unsigned int current_opacity;
    select_method_t select_method = SELECT_METHOD_CLICK;
    Bool flag_toggle = False;
    Bool flag_increase = False;
    Bool flag_decrease = False;
    Bool flag_verbose = False;
    Bool flag_no_regex = False;
    int o;
    double min = 0.0, max = 1.0;
    char *idstr = NULL, *namestr = NULL;
    char *windowname = NULL;

    int options_index = 0;
    static struct option long_options[] = {
        {"toggle", 0, NULL, 't'},
        {"help", 0, NULL, 'h'},
        {"point", 0, NULL, 'p'},
        {"actual", 0, NULL, 'a'},
        {"click", 0, NULL, 'c'},
        {"id", 1, NULL, 'i'},
        {"name", 1, NULL, 'n'},
        {"inc", 0, NULL, '1'},
        {"dec", 0, NULL, '2'},
        {"min", 1, NULL, 'm'},
        {"max", 1, NULL, 'x'},
        {"no-regex", 0, NULL, '4'},
        {"version", 0, NULL, 'V'},
        {"verbose", 0, NULL, 'v'},
        {0, 0, 0, 0}
    };
    unsigned char *data;

    Atom actual;
    int format;
    unsigned long n, left;

    /* wonderful utility */
    Setup_Display_And_Screen (&argc, argv);

    /* parse arguments */
    while ((o = getopt_long (argc, argv, "thapci:n:vVm:x:123",
                             long_options, &options_index)) != -1) {
        switch (o) {
        case 't':
            flag_toggle = True;
            break;
        case 'h':
            Usage ();
            break;
        case 'c':
            select_method = SELECT_METHOD_CLICK;
            break;
        case 'p':
            select_method = SELECT_METHOD_WINDOW_UNDER_CURSOR;
            break;
        case 'i':
            idstr = optarg;
            select_method = SELECT_METHOD_WINDOW_ID;
            break;
        case 'n':
            namestr = optarg;
            select_method = SELECT_METHOD_WINDOW_NAME;
            break;
        case 'a':
            select_method = SELECT_METHOD_FOCUSED_WINDOW;
            break;
        case '1':
            flag_increase = True;
            break;
        case '2':
            flag_decrease = True;
            break;
        case 'v':
            flag_verbose = True;
            break;
        case '4':
            flag_no_regex = True;
            break;
        case 'm':
            min = atof (optarg);
            break;
        case 'x':
            max = atof (optarg);
            break;
        case 'V':
            fprintf (stderr, "%s\nversion: %s\nreleased: %s\n",
                     PACKAGE_STRING, VERSIONSTR, RELEASE_DATE);
            exit (1);
            break;
        default:
            Usage ();
        }
    }

    if (optind < argc) {
        d = atof (argv[optind]);
        gotd = True;
    }

    /* select the window to make transparent */
    switch (select_method) {
    case SELECT_METHOD_WINDOW_UNDER_CURSOR:
        /* don't wait for click */
        if (flag_verbose)
            printf ("Selecting window by click\n");
        target_win = Get_Window_Under_Cursor (dpy);
        break;

    case SELECT_METHOD_WINDOW_ID:
        /* pretty much ripped from dsimple.c */
        if (flag_verbose)
            printf ("Selecting window by id\n");
        sscanf (idstr, "0x%lx", &target_win);
        if (!target_win)
            sscanf (idstr, "%ld", &target_win);
        if (!target_win) {
            fprintf (stderr, "Invalid window id format: %s.\n", idstr);
            XCloseDisplay (dpy);
            return 1;
        }
        if (flag_verbose)
            printf ("Selected 0x%x, trying to get top parent ... ",
                    (unsigned int) target_win);
        target_win = Get_Top_Window (dpy, target_win);
        if (flag_verbose)
            printf ("found 0x%x\n", (unsigned int) target_win);

        break;

    case SELECT_METHOD_WINDOW_NAME:
        /* select by name, pretty much ripped from dsimple.c */
        if (flag_verbose)
            printf ("Selecting window by name\n");

        if (flag_no_regex)
            target_win = Window_With_Name (dpy, RootWindow (dpy, screen),
                                           namestr);
        else
            target_win = Window_With_Name_Regex (dpy, RootWindow (dpy, screen),
                                                 namestr);

        if (!target_win) {
            fprintf (stderr, "No window matching %s exists!\n", namestr);
            XCloseDisplay (dpy);
            return 1;
        }
        /* store the matched window name*/
        XFetchName (dpy, target_win, &windowname);

        if (flag_verbose)
            printf ("Selected 0x%x, trying to get top parent ... ",
                    (unsigned int) target_win);

        target_win = Get_Top_Window (dpy, target_win);
        if (flag_verbose)
            printf ("found 0x%x\n", (unsigned int) target_win);

        break;

    case SELECT_METHOD_FOCUSED_WINDOW:
        target_win = Get_Actual_Window (dpy);
        break;

    default:
        /* grab mouse and return window that is next clicked */
        target_win = Select_Window (dpy);
        break;
    }

    /* get property */
    if ((XGetWindowProperty (dpy, target_win, XInternAtom (dpy, OPACITY, False),
                             0L, 1L, False, XA_CARDINAL, &actual, &format, &n,
                             &left, &data) == Success)
        && (data != None)) {
        memcpy (&current_opacity, data, sizeof (unsigned int));
        XFree ((void *) data);
        if (flag_verbose)
            printf ("Found transparency: %g\n",
                    (double) current_opacity / OPAQUE);
    } else
        current_opacity = OPAQUE;

    if (flag_increase)
        d = (double) current_opacity / OPAQUE + d;
    else if (flag_decrease)
        d = (double) current_opacity / OPAQUE - d;

    /* check min and max */
    if (d < min)
        d = min;
    if (d > max)
        d = max;

    opacity = (unsigned int) (d * OPAQUE);

    /* for user-compatibility with transset */
    if (!gotd)
        flag_toggle = True;

    /* toggle */
    if (flag_toggle)
        if (current_opacity != OPAQUE)
            opacity = OPAQUE;

    if (opacity == OPAQUE)
  	XDeleteProperty (dpy, target_win, XInternAtom (dpy, OPACITY, False));
    /* set it */
    else
        XChangeProperty (dpy, target_win, XInternAtom (dpy, OPACITY, False),
                         XA_CARDINAL, 32, PropModeReplace,
                         (unsigned char *) &opacity, 1L);
    XSync (dpy, False);

    printf ("Set Property to %g", (double) opacity / OPAQUE);
    if (windowname)
        printf (" on \n%s\n", windowname);
    else
        printf ("\n");

    if (flag_verbose)
        printf ("Property set on: 0x%x\n", (unsigned int) target_win);

    XCloseDisplay (dpy);
    return 0;
}
