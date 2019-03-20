#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

int main(void)
{
	Display *dpy;
	XF86VidModeModeLine current;
	XF86VidModeModeInfo **modes;
	int num_modes, i;
	int saved_mode = -1;
	int dotclock;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		dpy = XOpenDisplay(":0");

	XF86VidModeGetModeLine(dpy, DefaultScreen(dpy), &dotclock, &current);
	XF86VidModeGetAllModeLines(dpy, XDefaultScreen(dpy),
				   &num_modes, &modes);
	for (i = 0; i < num_modes; i++) {
		int this;

		this = (current.hdisplay == modes[i]->hdisplay &&
			current.vdisplay == modes[i]->vdisplay &&
			dotclock == modes[i]->dotclock);
		if (this && saved_mode == -1)
			saved_mode = i;

		printf("[%d] %dx%d%s\n",
		       i,
		       modes[i]->hdisplay,
		       modes[i]->vdisplay,
		       this ? "*" : "");
	}

	for (i = 0; i < num_modes; i++) {
		printf("Switching to mode %dx%d\n",
		       modes[i]->hdisplay,
		       modes[i]->vdisplay);
		XF86VidModeSwitchToMode(dpy, XDefaultScreen(dpy), modes[i]);
		XSync(dpy, True);
	}

	if (saved_mode != -1) {
		XF86VidModeSwitchToMode(dpy, XDefaultScreen(dpy),
					modes[saved_mode]);
		XFlush(dpy);
	}

	return 0;
}
