/* cc -O2 -o xftlsfonts xftlsfonts.c -I/usr/X11R6/include -L/usr/X11R6/lib -R/usr/X11R6/lib -lXft -lX11 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

char           *program_name;

void
usage()
{
	fprintf(stderr, "usage:  %s [-fn name]\n", program_name);
	fprintf(stderr, "\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	register int    i;
	Display        *display;
	int             screen;
	XftPattern     *pattern = NULL;
	XftObjectSet   *os;
	XftFontSet     *font_set;

	program_name = argv[0];
	if ((display = XOpenDisplay(NULL)) == NULL) {
		(void) fprintf(stderr, "Cannot open Display.\n");
		exit(1);
	}
	screen = DefaultScreen(display);
	for (argv++, argc--; argc; argv++, argc--) {
		if (argv[0][0] == '-') {
			for (i = 1; argv[0][i]; i++) {
				switch (argv[0][i]) {
				case 'f':
					argc--;
					if (argc <= 0) {
						usage();
					}
					argv++;
					pattern = XftNameParse(argv[0]);
					goto next;
				default:
					usage();
					break;
				}
			}
		} else {
			pattern = XftNameParse(argv[0]);
		}
next:		;
	}
	if (pattern == NULL) {
		pattern = XftPatternCreate();
	}
	os = XftObjectSetCreate();
	XftObjectSetAdd(os, XFT_FAMILY);
	XftObjectSetAdd(os, XFT_STYLE);
#if 0
	XftObjectSetAdd(os, XFT_SPACING);
	XftObjectSetAdd(os, XFT_FOUNDRY);
#endif
	XftObjectSetAdd(os, XFT_XLFD);
	XftObjectSetAdd(os, XFT_FILE);
	font_set = XftListFontsPatternObjects(display, screen, pattern, os);
	XftObjectSetDestroy(os);
	XftPatternDestroy(pattern);
	if (font_set != NULL) {
		XftFontSetPrint(font_set);
		XftFontSetDestroy(font_set);
	}
	XCloseDisplay(display);
	return 0;
}
