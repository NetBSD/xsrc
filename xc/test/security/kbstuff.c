#include <stdio.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>

int
catch_error(dpy, errevent)
Display *dpy;
XErrorEvent *errevent;
{
    char buf[80];
    XGetErrorText(dpy, errevent->error_code, buf, sizeof (buf));
    printf("major op %d error %d %s\n", errevent->request_code, errevent->error_code, buf);
    return 1;
}

char *grab_stat_strings[] = {
    "GrabSuccess", "AlreadyGrabbed", "GrabInvalidTime", "GrabNotViewable",
    "GrabFrozen"
};

char *ProgramName;

void usage()
{
    printf("%s -display name -ownerevents 0|1 -eventmask integer\n",
	   ProgramName);
}

int main(argc, argv)
    int argc;
    char **argv;
{
    int i;
    char *displayname = NULL;
    Display *dpy;
    int status;
    Window grab_window;
    Bool owner_events;
    int pointer_mode;
    int keyboard_mode;
    Time time;
    XSetWindowAttributes swa;
    int screen;
    Visual *visual;
    int depth;
    Window root;

    swa.event_mask = KeyPressMask | KeyReleaseMask;
    owner_events  = False;
    ProgramName = argv[0];

    for (i = 1; i < argc; i++)
    {
	char *arg = argv[i];

	if (!strcmp("-display", arg))
	{
	    if (++i >= argc) usage ();
	    displayname = argv[i];
	}
	else if (!strcmp("-eventmask", arg))
	{
	    if (++i >= argc) usage ();
	    swa.event_mask = atoi(argv[i]);
	}
	else if (!strcmp("-ownerevents", arg))
	{
	    if (++i >= argc) usage ();
	    owner_events = atoi(argv[i]);
	}
	else
	    usage();
    }

    displayname = XDisplayName(displayname);

    XSetErrorHandler(catch_error);
    dpy = XOpenDisplay(displayname);
    if (!dpy) return(1);
    XSynchronize(dpy, True);

    root = DefaultRootWindow(dpy);
    screen = DefaultScreen(dpy);
    visual = DefaultVisual(dpy, screen);
    depth = DefaultDepth(dpy, screen);

    swa.override_redirect = True;
    grab_window = XCreateWindow(dpy, root, 0, 0, 100, 100, 1, depth,
			InputOutput, visual, CWEventMask|CWOverrideRedirect,
				&swa);
    XMapWindow(dpy, grab_window);

    pointer_mode  = GrabModeAsync;
    keyboard_mode = GrabModeAsync;
    time = CurrentTime;

    for (;;)
    {
	while (XPending(dpy))
	{
	    XEvent event;
	    XNextEvent(dpy, &event);
	    if (event.type == KeyPress )
	    {
		KeySym ksym = XLookupKeysym((XKeyEvent *)&event, 0);
		char *s;
		s = XKeysymToString(ksym);
		printf("%s\n", s);
	    }
	}
	printf("Grabbing keyboard...");
	status = XGrabKeyboard(dpy, grab_window,
			       owner_events, pointer_mode, 
			       keyboard_mode, time);
	printf("%s ", grab_stat_strings[status]);
	fflush(stdout);
	sleep(5);
	printf("Ungrabbing keyboard\n");
	fflush(stdout);
	XUngrabKeyboard(dpy, CurrentTime);

	printf("Setting input focus...\n");
	XSetInputFocus(dpy, grab_window, RevertToPointerRoot, CurrentTime);
	fflush(stdout);
	sleep(5);

	printf("Grabbing keys...\n");
	fflush(stdout);
	XGrabKey(dpy, AnyKey, AnyModifier, grab_window, False,
		 GrabModeAsync, GrabModeAsync);
	sleep(5);
	printf("Ungrabbing keys...\n");
	fflush(stdout);
	XUngrabKey(dpy, AnyKey, AnyModifier, grab_window);
    }

    return 0;
}
