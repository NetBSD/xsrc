#include <stdio.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>

int bad_requests = 0;

int
catch_error(dpy, errevent)
Display *dpy;
XErrorEvent *errevent;
{
    char buf[80];
    XGetErrorText(dpy, errevent->error_code, buf, sizeof (buf));
    printf("major op %d error %d %s\n", errevent->request_code, errevent->error_code, buf);
    if (errevent->error_code == BadRequest) bad_requests++;
    return 1;
}

void
issue_probe_request(dpy, X_probe)
    Display *dpy;
    int X_probe;
{
    register xReq *req;

    LockDisplay(dpy);
    GetEmptyReq(probe, req);
    UnlockDisplay(dpy);
    SyncHandle();
}


int main(argc, argv)
    int argc;
    char **argv;
{
    int i;
    char *display_env = NULL;
    Display *dpy;

    if (argc >= 2  &&  strcmp(argv[1], "-display") == 0)
	display_env = argv[2];

    XSetErrorHandler(catch_error);
    dpy = XOpenDisplay(display_env);
    if (!dpy) return(1);

    for (i = 128; i < 256; i++)
    {
	issue_probe_request(dpy, i);
	XSync(dpy, False);
    }
    XSync(dpy, False);
    XCloseDisplay(dpy);
    printf("%d extensions found\n", 128 - bad_requests);
    fflush(stdout);
    return 0;
}
