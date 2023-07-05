/*
 * Routines for un/masking the whole screen.
 *
 * Used in showing the WelcomeWindow splash screen.
 */

#include "ctwm.h"

#include <sys/select.h>

#include "screen.h"
#include "icons.h"
#include "cursor.h"
#include "image.h"
#include "gram.tab.h"
#include "list.h"
#include "vscreen.h"
#include "win_decorations.h"
#include "win_utils.h"
#include "workspace_manager.h"

#include "mask_screen.h"


/* Various internal subbits */
static void PaintAllDecoration(void);


/*
 * Masking and unmasking; our public interface
 */
void
MaskScreen(char *file)
{
	unsigned long valuemask;
	XSetWindowAttributes attributes;
	XEvent event;
	Cursor waitcursor;
	int x, y;
	XColor black;

	NewFontCursor(&waitcursor, "watch");

	valuemask = (CWBackPixel | CWOverrideRedirect | CWEventMask | CWCursor);
	attributes.override_redirect = True;
	attributes.event_mask        = ExposureMask;
	attributes.cursor            = waitcursor;
	attributes.background_pixel  = Scr->Black;
	Scr->WindowMask = XCreateWindow(dpy, Scr->Root, 0, 0,
	                                Scr->rootw,
	                                Scr->rooth,
	                                0,
	                                CopyFromParent, CopyFromParent,
	                                CopyFromParent, valuemask,
	                                &attributes);
	XMapWindow(dpy, Scr->WindowMask);
	XMaskEvent(dpy, ExposureMask, &event);

	if(Scr->Monochrome != COLOR) {
		return;
	}

	ColorPair WelcomeCp = {
		.fore = Scr->Black,
		.back = Scr->White,
	};
	Scr->WelcomeCmap  = XCreateColormap(dpy, Scr->WindowMask, Scr->d_visual,
	                                    AllocNone);
	if(! Scr->WelcomeCmap) {
		return;
	}
	XSetWindowColormap(dpy, Scr->WindowMask, Scr->WelcomeCmap);
	black.red   = 0;
	black.green = 0;
	black.blue  = 0;
	XAllocColor(dpy, Scr->WelcomeCmap, &black);

	AlternateCmap = Scr->WelcomeCmap;
	if(! file) {
		Scr->WelcomeImage  = GetImage("xwd:welcome.xwd", WelcomeCp);
#ifdef XPM
		if(Scr->WelcomeImage == None) {
			Scr->WelcomeImage  = GetImage("xpm:welcome.xpm", WelcomeCp);
		}
#endif
	}
	else {
		Scr->WelcomeImage  = GetImage(file, WelcomeCp);
	}
	AlternateCmap = None;
	if(Scr->WelcomeImage == None) {
		return;
	}

	if(0) {
		// Dummy
	}
#ifdef CAPTIVE
	else if(CLarg.is_captive) {
		XSetWindowColormap(dpy, Scr->WindowMask, Scr->WelcomeCmap);
		XSetWMColormapWindows(dpy, Scr->Root, &(Scr->WindowMask), 1);
	}
#endif
	else {
		XInstallColormap(dpy, Scr->WelcomeCmap);
	}

	Scr->WelcomeGC = XCreateGC(dpy, Scr->WindowMask, 0, NULL);
	x = (Scr->rootw  -  Scr->WelcomeImage->width) / 2;
	y = (Scr->rooth - Scr->WelcomeImage->height) / 2;

	XSetWindowBackground(dpy, Scr->WindowMask, black.pixel);
	XClearWindow(dpy, Scr->WindowMask);
	XCopyArea(dpy, Scr->WelcomeImage->pixmap, Scr->WindowMask, Scr->WelcomeGC, 0, 0,
	          Scr->WelcomeImage->width, Scr->WelcomeImage->height, x, y);
}



void
UnmaskScreen(void)
{
	struct timeval      timeout;
	Colormap            stdcmap = Scr->RootColormaps.cwins[0]->colormap->c;
	Colormap            cmap;
	XColor              colors [256], stdcolors [256];
	int                 i, j, usec;

	usec = 6000;
	timeout.tv_usec = usec % (unsigned long) 1000000;
	timeout.tv_sec  = usec / (unsigned long) 1000000;

	if(Scr->WelcomeImage) {
		Pixel pixels [256];

		cmap = Scr->WelcomeCmap;
		for(i = 0; i < 256; i++) {
			pixels [i] = i;
			colors [i].pixel = i;
		}
		XQueryColors(dpy, cmap, colors, 256);
		XFreeColors(dpy, cmap, pixels, 256, 0L);
		XFreeColors(dpy, cmap, pixels, 256, 0L);   /* Ah Ah */

		for(i = 0; i < 256; i++) {
			colors [i].pixel = i;
			colors [i].flags = DoRed | DoGreen | DoBlue;
			stdcolors [i].red   = colors [i].red;
			stdcolors [i].green = colors [i].green;
			stdcolors [i].blue  = colors [i].blue;
		}
		for(i = 0; i < 128; i++) {
			for(j = 0; j < 256; j++) {
				colors [j].red   = stdcolors [j].red   * ((127.0 - i) / 128.0);
				colors [j].green = stdcolors [j].green * ((127.0 - i) / 128.0);
				colors [j].blue  = stdcolors [j].blue  * ((127.0 - i) / 128.0);
			}
			XStoreColors(dpy, cmap, colors, 256);
			select(0, NULL, NULL, NULL, &timeout);
		}
		XFreeColors(dpy, cmap, pixels, 256, 0L);
		XFreeGC(dpy, Scr->WelcomeGC);
		FreeImage(Scr->WelcomeImage);
	}
	if(Scr->Monochrome != COLOR) {
		goto fin;
	}

	cmap = XCreateColormap(dpy, Scr->Root, Scr->d_visual, AllocNone);
	if(! cmap) {
		goto fin;
	}
	for(i = 0; i < 256; i++) {
		colors [i].pixel = i;
		colors [i].red   = 0;
		colors [i].green = 0;
		colors [i].blue  = 0;
		colors [i].flags = DoRed | DoGreen | DoBlue;
	}
	XStoreColors(dpy, cmap, colors, 256);

	if(0) {
		// Dummy
	}
#ifdef CAPTIVE
	else if(CLarg.is_captive) {
		XSetWindowColormap(dpy, Scr->Root, cmap);
	}
#endif
	else {
		XInstallColormap(dpy, cmap);
	}

	XUnmapWindow(dpy, Scr->WindowMask);
	XClearWindow(dpy, Scr->Root);
	XSync(dpy, 0);
	PaintAllDecoration();

	for(i = 0; i < 256; i++) {
		stdcolors [i].pixel = i;
	}
	XQueryColors(dpy, stdcmap, stdcolors, 256);
	for(i = 0; i < 128; i++) {
		for(j = 0; j < 256; j++) {
			colors [j].pixel = j;
			colors [j].red   = stdcolors [j].red   * (i / 127.0);
			colors [j].green = stdcolors [j].green * (i / 127.0);
			colors [j].blue  = stdcolors [j].blue  * (i / 127.0);
			colors [j].flags = DoRed | DoGreen | DoBlue;
		}
		XStoreColors(dpy, cmap, colors, 256);
		select(0, NULL, NULL, NULL, &timeout);
	}

	if(0) {
		// Dummy
	}
#ifdef CAPTIVE
	else if(CLarg.is_captive) {
		XSetWindowColormap(dpy, Scr->Root, stdcmap);
	}
#endif
	else {
		XInstallColormap(dpy, stdcmap);
	}

	XFreeColormap(dpy, cmap);

fin:
	if(Scr->WelcomeCmap) {
		XFreeColormap(dpy, Scr->WelcomeCmap);
	}
	XDestroyWindow(dpy, Scr->WindowMask);
	Scr->WindowMask = (Window) 0;
}




/*
 * Internal utils
 */
static void
PaintAllDecoration(void)
{
	TwmWindow *tmp_win;
	VirtualScreen *vs;

	for(tmp_win = Scr->FirstWindow; tmp_win != NULL; tmp_win = tmp_win->next) {
		if(! visible(tmp_win)) {
			continue;
		}
		if(tmp_win->mapped) {
			if(tmp_win->frame_bw3D) {
				PaintBorders(tmp_win,
				             (tmp_win->highlight && tmp_win == Scr->Focus));
			}
			if(tmp_win->title_w) {
				PaintTitle(tmp_win);
			}
			if(tmp_win->titlebuttons) {
				PaintTitleButtons(tmp_win);
			}
		}
		else if((tmp_win->icon_on == true)  &&
		                !Scr->NoIconTitlebar    &&
		                tmp_win->icon           &&
		                tmp_win->icon->w        &&
		                !tmp_win->icon->w_not_ours &&
		                ! LookInList(Scr->NoIconTitle, tmp_win->name, &tmp_win->class)) {
			PaintIcon(tmp_win);
		}
	}
	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		PaintWorkSpaceManager(vs);
	}
}
