/*
 * $XFree86: xc/programs/xrandr/xrandr.c,v 1.8 2001/07/11 16:40:32 keithp Exp $
 *
 * Copyright © 2001 Keith Packard, member of The XFree86 Project, Inc.
 * Copyright © 2001 Compaq Computer Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard or Compaq not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD and COMPAQ DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * Blame Jim Gettys for any bugs...
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/Xrandr.h>
#include <string.h>
#include <stdlib.h>

static char *program_name;

static char *direction[5] = {
  "normal", 
  "left", 
  "inverted", 
  "right",
  "\n"};


static void
usage(void)
{
  fprintf(stderr, "usage: %s [options]\n", program_name);
  fprintf(stderr, "  where options are:\n");
  fprintf(stderr, "  -display <display> or -d <display>\n");
  fprintf(stderr, "  -help\n");
  fprintf(stderr, "  -o <normal,inverted,left,right,0,1,2,3>\n");
  fprintf(stderr, "   or --orientation <normal,inverted,left,right,0,1,2,3>\n");
  fprintf(stderr, "  -q                or --query\n");
  fprintf(stderr, "  -s <size>         or --size <size>\n");
  fprintf(stderr, "  -v                or --verbose\n");
  fprintf(stderr, "                    or --screen <screen>\n");
  
  exit(1);
  /*NOTREACHED*/
}

int
main (int argc, char **argv)
{
  Display       *dpy;
  XRRScreenSize *sizes;
  XRRScreenConfiguration *sc;
  int		nsize;
  Window	root;
  Status	status = RRSetConfigFailed;
  int		rot = -1;
  int		verbose = 0, query = 0;
  Rotation	rotation, current_rotation, rotations;
  XRRScreenChangeNotifyEvent event;    
  char          *display_name = NULL;
  int 		i;
  SizeID	current_size;
  VisualGroupID	current_visual_group;
  int		size = -1;
  int		dirind = 0;
  int		setit = 0;
  int		screen = -1;

  program_name = argv[0];
  if (argc == 1) query = 1;
  for (i = 1; i < argc; i++) {
    if (!strcmp ("-display", argv[i]) || !strcmp ("-d", argv[i])) {
      if (++i>=argc) usage ();
      display_name = argv[i];
      continue;
    }
    if (!strcmp("-help", argv[i])) {
      usage();
    }
    if (!strcmp ("-v", argv[i]) || !strcmp ("--verbose", argv[i])) {
      verbose = 1;
      continue;
    }
    if (!strcmp ("-s", argv[i]) || !strcmp ("--size", argv[i])) {
      if (++i>=argc) usage ();
      size = atoi (argv[i]);
      if (size < 0) usage();
      setit = 1;
      continue;
    }
    if (!strcmp ("--screen", argv[i])) {
      if (++i>=argc) usage ();
      screen = atoi (argv[i]);
      if (screen < 0) usage();
      continue;
    }
    if (!strcmp ("-q", argv[i]) || !strcmp ("--query", argv[i])) {
      query = 1;
      continue;
    }
    if (!strcmp ("-o", argv[i]) || !strcmp ("--orientation", argv[i])) {
      char *endptr;
      if (++i>=argc) usage ();
      dirind = strtol(argv[i], &endptr, 0);
      if (*endptr != '\0') {
	for (dirind = 0; dirind < 4; dirind++) {
	  if (strcmp (direction[dirind], argv[i]) == 0) break;
	}
	if ((dirind < 0) || (dirind > 3))  usage();
      }
      rot = dirind;
      setit = 1;
      continue;
    }
    usage();
  }
  if (verbose) query = 1;

  dpy = XOpenDisplay (display_name);
  if (dpy == NULL) {
      fprintf (stderr, "Can't open display %s\n", display_name);
      exit (1);
  }
  if (screen < 0)
    screen = DefaultScreen (dpy);
  if (screen >= ScreenCount (dpy)) {
    fprintf (stderr, "Invalid screen number %d (display has %d)\n",
	     screen, ScreenCount (dpy));
    exit (1);
  }

  root = RootWindow (dpy, screen);

  sc = XRRGetScreenInfo (dpy, root);

  if (sc == NULL) 
      exit (1);
  
  current_size = XRRCurrentConfig (sc, &current_visual_group, &current_rotation);
  if (size < 0)
    size = current_size;
  if (rot < 0)
  {
    for (rot = 0; rot < 4; rot++)
	if (1 << rot == current_rotation)
	    break;
  }

  sizes = XRRSizes(sc, &nsize);
  for (i = 0; i < nsize; i++) {
    if (query) 
      printf ("SZ:  Pixels        Physical\n%d: %4d x%4d  (%4dmm x%4dmm)\n",
	      i, sizes[i].width, sizes[i].height,
	      sizes[i].mwidth, sizes[i].mheight);
  }

  if (size >= nsize) usage();
  rotations = XRRRotations(sc, &current_rotation);
  rotation = 1 << rot ;
  if (query) {
    for (i = 0; i < 4; i ++) {
      if ((current_rotation >> i) & 1) 
	printf("Current rotation - %s\n", direction[i]);
    }
    printf ("Rotations possible - ");
    for (i = 0; i < 4; i ++) {
      if ((rotations >> i) & 1)  printf("%s ", direction[i]);
    }
    printf ("\n");
  }

  if (verbose) printf("Setting size to %d, rotation to %s\n", 
		      size, direction[rot]);

  if (setit) XRRScreenChangeSelectInput (dpy, root, True);
  if (setit) status = XRRSetScreenConfig (dpy, sc, DefaultRootWindow (dpy), 
		       (SizeID) size, current_visual_group, (Rotation) rotation, CurrentTime);
  if (verbose && setit) {
    if (status == RRSetConfigSuccess)
      {
	XNextEvent(dpy, (XEvent *) &event);
	printf("Got an event!\n");
	printf(" window = %d\n root = %d\n size_index = %d\n visual_group_index %d\n rotation %d\n", 
	       (int) event.window, (int) event.root, 
	       event.size_index, event.visual_group_index, event.rotation);
	printf(" timestamp = %ld, config_timestamp = %ld\n",
	       event.timestamp, event.config_timestamp);
	printf(" %dX%d pixels, %dX%d mm\n",
	       event.width, event.height,
	       event.mwidth, event.mheight);
      }
  }
  XRRFreeScreenInfo(sc);
  return(0);
}

