/*
 * Forward key and mouse button events from one window to another.
 *
 * This is useful to use a Desktop window as a Root window.
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>

char *ProgramName;
Display *dpy;
int screen;

static void
usage(void)
{
	static const char *msg[] = {
		"    -display displayname                X server to contact",
		"    -id windowid                        use existing window",
		"    -root                               use root window",
		"    -name string                        window name",
		"",
		NULL
	};
	const char **cpp;

	fprintf(stderr, "usage:  %s [-options ...]\n", ProgramName);
	fprintf(stderr, "where options include:\n");

	for(cpp = msg; *cpp; cpp++) {
		fprintf(stderr, "%s\n", *cpp);
	}

	exit(1);
}

int
main(int argc, char **argv)
{
	char *displayname = NULL;
	int i;
	Window w;
	XWindowAttributes wattr;
	long event_mask;
	int done;
	Window ws[2];   /* from-window, to-window */
	int wi = 0;
	char *name;

	ProgramName = argv[0];

	if(setlocale(LC_ALL, "") == NULL) {
		fprintf(stderr, "%s: warning: could not set default locale\n",
		        ProgramName);
	}

	w = 0;
	for(i = 1; i < argc; i++) {
		char *arg = argv[i];

		if(arg[0] == '-') {
			switch(arg[1]) {
				case 'd':                 /* -display host:dpy */
					if(++i >= argc) {
						usage();
					}
					displayname = argv[i];
					continue;
				case 'i':                 /* -id */
					if(++i >= argc || wi > 1) {
						usage();
					}
					sscanf(argv[i], "0x%lx", &w);
					if(!w) {
						sscanf(argv[i], "%lu", &w);
					}
					if(!w) {
						usage();
					}
					ws[wi++] = w;
					continue;
				case 'n':                 /* -name */
					if(++i >= argc || wi > 1) {
						usage();
					}
					name = argv[i]; /* not implemented yet */
					(void)name; // Silence unused warning
					continue;
				case 'r':
					switch(arg[2]) {
						case 'o':             /* -root */
							if(wi > 1) {
								usage();
							}
							ws[wi++] = -1;
							continue;
						default:
							usage();
					}
					continue;
				default:
					usage();
			}                           /* end switch on - */
		}
		else {
			usage();
		}
	}                                   /* end for over argc */

	dpy = XOpenDisplay(displayname);
	if(!dpy) {
		fprintf(stderr, "%s:  unable to open display '%s'\n",
		        ProgramName, XDisplayName(displayname));
		exit(1);
	}
	screen = DefaultScreen(dpy);

	if(ws[0] == -1) {
		ws[0] = RootWindow(dpy, screen);
	}
	if(ws[1] == -1) {
		ws[1] = RootWindow(dpy, screen);
	}

	if(ws[0] == ws[1]) {
		fprintf(stderr, "error: from-window and to-window must differ.\n");
		exit(1);
	}

	event_mask = KeyPressMask | KeyReleaseMask |
	             ButtonPressMask | ButtonReleaseMask;

	if(ws[0]) {
		XGetWindowAttributes(dpy, ws[0], &wattr);
		/* We can't select on button presses if someone else already does... */
		if(wattr.all_event_masks & ButtonPressMask) {
			event_mask &= ~ButtonPressMask;
			fprintf(stderr, "warning: can't forward button presses.\n");
		}
		event_mask &= ~SubstructureRedirectMask;
		printf("XSelectInput 0x%x %lx\n", (unsigned int)ws[0], event_mask);
		XSelectInput(dpy, ws[0], event_mask);
	}

	for(done = 0; !done;) {
		XEvent event;

		XNextEvent(dpy, &event);

		switch(event.type) {
			case KeyPress:
			case KeyRelease:
				printf("KeyPress/KeyRelease\n");
				event.xkey.window = ws[1];
				XSendEvent(dpy, ws[1], False, KeyPressMask, &event);
				break;
			case ButtonPress:
			case ButtonRelease:
				printf("ButtonPress/ButtonRelease\n");
				event.xbutton.window = ws[1];
				XSendEvent(dpy, ws[1], False, KeyPressMask, &event);
				break;
			default:
				printf("some other event\n");
		}
	}

	XCloseDisplay(dpy);
	return 0;
}
