/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * $XConsortium: twm.c,v 1.124 91/05/08 11:01:54 dave Exp $
 *
 * twm - "Tom's Window Manager"
 *
 * 27-Oct-87 Thomas E. LaStrange        File created
 * 10-Oct-90 David M. Sternlicht        Storing saved colors on root
 *
 * Copyright 1992 Claude Lecommandeur.
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <fcntl.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>


#include "ctwm_atoms.h"
#include "ctwm_main.h"
#include "ctwm_takeover.h"
#include "clargs.h"
#include "add_window.h"
#include "gc.h"
#include "parse.h"
#include "version.h"
#include "colormaps.h"
#include "events.h"
#include "util.h"
#include "mask_screen.h"
#include "animate.h"
#include "screen.h"
#include "icons.h"
#include "iconmgr.h"
#include "list.h"
#ifdef SESSION
#include "session.h"
#endif
#include "occupation.h"
#include "otp.h"
#include "cursor.h"
#ifdef WINBOX
#include "windowbox.h"
#endif
#ifdef CAPTIVE
#include "captive.h"
#endif
#ifdef XRANDR
#include "xrandr.h"
#endif
#include "r_area.h"
#include "r_area_list.h"
#include "r_layout.h"
#include "signals.h"
#include "vscreen.h"
#include "win_decorations_init.h"
#include "win_ops.h"
#include "win_regions.h"
#include "win_utils.h"
#include "workspace_manager.h"
#ifdef SOUNDS
#  include "sound.h"
#endif

#include "gram.tab.h"


XtAppContext appContext;        /* Xt application context */
Display *dpy;                   /* which display are we talking to */
Window ResizeWindow;            /* the window we are resizing */

Atom XCTWMAtom[NUM_CTWM_XATOMS]; ///< Our various common atoms

int NumScreens;                 /* number of screens in ScreenList */
bool HasShape;                  /* server supports shape extension? */
int ShapeEventBase, ShapeErrorBase;
ScreenInfo **ScreenList;        /* structures for each screen */
ScreenInfo *Scr = NULL;         /* the cur and prev screens */
int PreviousScreen;             /* last screen that we were on */
static bool cfgerrs = false;    ///< Whether there were config parsing errors

#ifdef CAPTIVE
static Window CreateCaptiveRootWindow(int x, int y,
                                      unsigned int width, unsigned int height);
#endif
ScreenInfo *InitScreenInfo(int scrnum, Window croot, int crootx, int crooty,
                           unsigned int crootw, unsigned int crooth);
static bool MappedNotOverride(Window w);

Cursor  UpperLeftCursor;
Cursor  TopRightCursor,
        TopLeftCursor,
        BottomRightCursor,
        BottomLeftCursor,
        LeftCursor,
        RightCursor,
        TopCursor,
        BottomCursor;

Cursor RightButt;
Cursor MiddleButt;
Cursor LeftButt;

XContext TwmContext;            /* context for twm windows */
XContext MenuContext;           /* context for all menu windows */
XContext ScreenContext;         /* context to get screen data */
XContext ColormapContext;       /* context for colormap operations */

XClassHint NoClass;             /* for applications with no class */

XGCValues Gcv;

char *Home;                     /* the HOME environment variable */
int HomeLen;                    /* length of Home */

bool HandlingEvents = false;    /* are we handling events yet? */

/*
 * Various junk vars for xlib calls.  Many calls have to get passed these
 * pointers to return values into, but in a lot of cases we don't care
 * about some/all of them, and since xlib blindly derefs and stores into
 * them, we can't just pass NULL for the ones we don't care about.  So we
 * make this set of globals to use as standin.  These should never be
 * used or read in our own code; use real vars for the values we DO use
 * from the calls.
 */
Window JunkRoot, JunkChild;
int JunkX, JunkY;
unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth, JunkMask;

char *ProgramName;
size_t ProgramNameLen;
int Argc;
char **Argv;

bool RestartPreviousState = true;      /* try to restart in previous state */


/// Magic flag for tests.  Nothing else should touch this!
bool ctwm_test = false;

/// Magic callback for tests.  This will trigger right after config file
/// parsing if it's set, and then exit.  Nothing else should ever touch
/// this!
int (*ctwm_test_postparse)(void) = NULL;




/**
 * Start up ctwm.  This is effectively main(), just wrapped for various
 * unimportant reasons.
 */
int
ctwm_main(int argc, char *argv[])
{
	int numManaged, firstscrn, lastscrn;
	bool FirstScreen;
	bool takeover = true;
	bool nodpyok = false;

	setlocale(LC_ALL, "");

	ProgramName = argv[0];
	ProgramNameLen = strlen(ProgramName);
	Argc = argc;
	Argv = argv;


	/*
	 * Run consistency check.  This is mostly to keep devs from
	 * accidental screwups; if a user ever sees anything from this,
	 * something went very very wrong.
	 */
	chk_keytable_order();

	/*
	 * Parse-out command line args, and check the results.
	 */
	clargs_parse(argc, argv);
	clargs_check();
	/* If we get this far, it was all good */

	/* Some clargs mean we're not actually trying to take over the screen */
	if(CLarg.cfgchk) {
		takeover = false;
	}
#ifdef CAPTIVE
	if(CLarg.is_captive) {
		takeover = false;
	}
#endif

	/* And some mean we actually don't care if we lack an X server */
	if(CLarg.cfgchk) {
		nodpyok = true;
	}

	/* Support for tests: be ready to fake everything */
	if(ctwm_test) {
		takeover = false;
		nodpyok  = true;
	}


	/*
	 * Hook up signal handlers
	 */
	setup_signal_handlers();


	// Various bits of code care about $HOME
	Home = getenv("HOME");
	if(Home == NULL) {
		Home = "./";
	}
	HomeLen = strlen(Home);


	// XXX This is only used in AddWindow(), and is probably bogus to
	// have globally....
	NoClass.res_name = NoName;
	NoClass.res_class = NoName;


	/*
	 * Initialize our X connection and state bits.
	 */
	{
		int zero = 0; // Fakey

		XtToolkitInitialize();
		appContext = XtCreateApplicationContext();

		// Tests don't talk to a real X server.
		// XXX This needs revisiting if we ever get one that _does_.
		// We'll have to add another flag...
		if(!ctwm_test) {
			// Connect
			dpy = XtOpenDisplay(appContext, CLarg.display_name, "twm", "twm",
			                    NULL, 0, &zero, NULL);
		}

		// Failed?  Usually a problem, but somethings we allow faking...
		if(!dpy && !nodpyok) {
			fprintf(stderr, "%s:  unable to open display \"%s\"\n",
			        ProgramName, XDisplayName(CLarg.display_name));
			exit(1);
		}

		if(dpy && fcntl(ConnectionNumber(dpy), F_SETFD, FD_CLOEXEC) == -1) {
			fprintf(stderr,
			        "%s:  unable to mark display connection as close-on-exec\n",
			        ProgramName);
			exit(1);
		}

		if(!dpy && !ctwm_test) {
			// At least warn, except for tests
			fprintf(stderr, "%s: Can't connect to X server, proceeding anyway...\n",
			        ProgramName);
		}
	}


#ifdef SESSION
	// Load session stuff
	if(CLarg.restore_filename) {
		ReadWinConfigFile(CLarg.restore_filename);
	}
#endif


	if(dpy) {
		// Load up info about X extensions
		HasShape = XShapeQueryExtension(dpy, &ShapeEventBase, &ShapeErrorBase);

		// Allocate contexts/atoms/etc we use
		TwmContext = XUniqueContext();
		MenuContext = XUniqueContext();
		ScreenContext = XUniqueContext();
		ColormapContext = XUniqueContext();
		InitWorkSpaceManagerContext();

		// Load up our standard set of atoms
		XInternAtoms(dpy, XCTWMAtomNames, NUM_CTWM_XATOMS, False, XCTWMAtom);

		NumScreens = ScreenCount(dpy);
		PreviousScreen = DefaultScreen(dpy);
	}
	else {
		NumScreens = 1;
		PreviousScreen = 0;
	}

	// Allocate/define common cursors
	NewFontCursor(&TopLeftCursor, "top_left_corner");
	NewFontCursor(&TopRightCursor, "top_right_corner");
	NewFontCursor(&BottomLeftCursor, "bottom_left_corner");
	NewFontCursor(&BottomRightCursor, "bottom_right_corner");
	NewFontCursor(&LeftCursor, "left_side");
	NewFontCursor(&RightCursor, "right_side");
	NewFontCursor(&TopCursor, "top_side");
	NewFontCursor(&BottomCursor, "bottom_side");

	NewFontCursor(&UpperLeftCursor, "top_left_corner");
	NewFontCursor(&RightButt, "rightbutton");
	NewFontCursor(&LeftButt, "leftbutton");
	NewFontCursor(&MiddleButt, "middlebutton");


	// Prep up the per-screen global info
	if(CLarg.MultiScreen) {
		firstscrn = 0;
		lastscrn = NumScreens - 1;
	}
	else if(!dpy) {
		firstscrn = lastscrn = 0;
	}
	else {
		firstscrn = lastscrn = DefaultScreen(dpy);
	}

	// For simplicity, pre-allocate NumScreens ScreenInfo struct pointers
	ScreenList = calloc(NumScreens, sizeof(ScreenInfo *));
	if(ScreenList == NULL) {
		fprintf(stderr, "%s: Unable to allocate memory for screen list, exiting.\n",
		        ProgramName);
		exit(1);
	}


	// Do a little early initialization
#ifdef EWMH
	if(dpy) {
		EwmhInit();
	}
#endif /* EWMH */
#ifdef SOUNDS
	// Needs init'ing before we get to config parsing
	sound_init();
#endif
	InitEvents();



	// Start looping over the screens
	numManaged = 0;
	FirstScreen = true;
	for(int scrnum = firstscrn ; scrnum <= lastscrn; scrnum++) {
		Window croot;
		int crootx, crooty;
		unsigned int crootw, crooth;
		bool screenmasked;
		char *welcomefile;


		/*
		 * First, setup the root window for the screen.
		 */
		if(0) {
			// Dummy
		}
#ifdef CAPTIVE
		else if(CLarg.is_captive) {
			// Captive ctwm.  We make a fake root.
			XWindowAttributes wa;
			if(CLarg.capwin && XGetWindowAttributes(dpy, CLarg.capwin, &wa)) {
				Window junk;
				croot  = CLarg.capwin;
				crootw = wa.width;
				crooth = wa.height;
				XTranslateCoordinates(dpy, CLarg.capwin, wa.root, 0, 0, &crootx, &crooty,
				                      &junk);
			}
			else {
				// Fake up default size.  Probably ideally should be
				// configurable, but even more ideally we wouldn't have
				// captive...
				crootx = crooty = 100;
				crootw = 1280;
				crooth = 768;
				croot = CreateCaptiveRootWindow(crootx, crooty, crootw, crooth);
			}
		}
#endif
		else {
			// Normal; get the real display's root.
			crootx = 0;
			crooty = 0;

			if(dpy) {
				croot  = RootWindow(dpy, scrnum);
				crootw = DisplayWidth(dpy, scrnum);
				crooth = DisplayHeight(dpy, scrnum);
			}
			else {
				croot = None;
				crootw = 1280;
				crooth = 768;
			}
		}



		/*
		 * Create ScreenInfo for this Screen, and populate various
		 * default/initial config.
		 */
		Scr = ScreenList[scrnum] = InitScreenInfo(scrnum, croot,
		                           crootx, crooty, crootw, crooth);
		if(Scr == NULL) {
			fprintf(stderr,
			        "%s: unable to allocate memory for ScreenInfo structure"
			        " for screen %d.\n",
			        ProgramName, scrnum);
			continue;
		}

		// Other misc adjustments to default config.
		Scr->ShowWelcomeWindow = CLarg.ShowWelcomeWindow;



		/*
		 * Figure out the layout of our various monitors if RANDR is
		 * around and can tell us.
		 */
#ifdef XRANDR
		if(dpy) {
			Scr->Layout = XrandrNewLayout(dpy, Scr->XineramaRoot);
		}
#endif
		if(Scr->Layout == NULL) {
			// No RANDR, so as far as we know, the layout is just one
			// monitor with our full size.
			RArea *fs;
			RAreaList *fsl;

			fs = RAreaNewStatic(Scr->rootx, Scr->rooty, Scr->rootw, Scr->rooth);
			fsl = RAreaListNew(1, fs, NULL);
			Scr->Layout = RLayoutNew(fsl);
		}
#ifdef DEBUG
		fprintf(stderr, "Layout: ");
		RLayoutPrint(Scr->Layout);
#endif
		if(RLayoutNumMonitors(Scr->Layout) < 1) {
			fprintf(stderr, "Error: No monitors found on screen %d!\n", scrnum);
			continue;
		}



		// Now we can stash some info about the screen
		if(dpy) {
			Scr->d_depth = DefaultDepth(dpy, scrnum);
			Scr->d_visual = DefaultVisual(dpy, scrnum);
			Scr->RealRoot = RootWindow(dpy, scrnum);
			{
				// Stash these for m4
				Screen *tscr = ScreenOfDisplay(dpy, scrnum);
				Scr->mm_w = tscr->mwidth;
				Scr->mm_h = tscr->mheight;
			}
		}
		else {
			// Standin; fake the values we need in m4 parsing
			Scr->d_visual = calloc(1, sizeof(Visual));
			Scr->d_visual->bits_per_rgb = 8;
			Scr->d_visual->class = TrueColor;
		}


		// Now that we have d_depth...
		Scr->XORvalue = (((unsigned long) 1) << Scr->d_depth) - 1;

#ifdef CAPTIVE
		// Init captive bits.  We stick this name into m4 props, so do it
		// before config processing.
		if(CLarg.is_captive) {
			Scr->CaptiveRoot = croot;
			Scr->captivename = AddToCaptiveList(CLarg.captivename);
			if(Scr->captivename) {
				XmbSetWMProperties(dpy, croot,
				                   Scr->captivename, Scr->captivename,
				                   NULL, 0, NULL, NULL, NULL);
			}
		}
#endif


		// Init some colormap bits.  We need this before we get into the
		// config parsing, since various things in there poke into
		// colormaps.
		{
			// 1 on the root
			Scr->RootColormaps.number_cwins = 1;
			Scr->RootColormaps.cwins = malloc(sizeof(ColormapWindow *));
			Scr->RootColormaps.cwins[0] = CreateColormapWindow(Scr->Root, true,
			                              false);
			Scr->RootColormaps.cwins[0]->visibility = VisibilityPartiallyObscured;

			// Initialize storage for all maps the Screen can hold
			Scr->cmapInfo.cmaps = NULL;
			if(dpy) {
				Scr->cmapInfo.maxCmaps = MaxCmapsOfScreen(ScreenOfDisplay(dpy,
				                         Scr->screen));
			}
			Scr->cmapInfo.root_pushes = 0;
			InstallColormaps(0, &Scr->RootColormaps);

			// Setup which we're using
			Scr->StdCmapInfo.head = Scr->StdCmapInfo.tail
			                        = Scr->StdCmapInfo.mru = NULL;
			Scr->StdCmapInfo.mruindex = 0;
			if(dpy) {
				LocateStandardColormaps();
			}
		}


		// Are we monochrome?  Or do we care this millennium?
		if(CLarg.Monochrome || (dpy && DisplayCells(dpy, scrnum) < 3)) {
			Scr->Monochrome = MONOCHROME;
		}
		else {
			Scr->Monochrome = COLOR;
		}


		// With the colormap/monochrome bits set, we can setup our
		// default color bits.
		GetColor(Scr->Monochrome, &(Scr->Black), "black");
		GetColor(Scr->Monochrome, &(Scr->White), "white");

		Scr->MenuShadowColor = Scr->Black;
		Scr->IconBorderColor = Scr->Black;
		Scr->IconManagerHighlight = Scr->Black;

#define SETFB(fld) Scr->fld.fore = Scr->Black; Scr->fld.back = Scr->White;
		SETFB(DefaultC)
		SETFB(BorderColorC)
		SETFB(BorderTileC)
		SETFB(TitleC)
		SETFB(MenuC)
		SETFB(MenuTitleC)
		SETFB(IconC)
		SETFB(IconManagerC)
		SETFB(workSpaceMgr.windowcp)
		SETFB(workSpaceMgr.curColors)
		SETFB(workSpaceMgr.defColors)
#undef SETFB


		// The first time around, we focus onto the root [of the first
		// Screen].  Maybe we should revisit this...
		if(dpy && FirstScreen) {
			// XXX This func also involves a lot of stuff that isn't
			// setup yet, and probably only works by accident.  Maybe we
			// should just manually extract out the couple bits we
			// actually want to run?
			SetFocus(NULL, CurrentTime);
			FirstScreen = false;
		}

		// Create default icon manager memory bits (in the first
		// workspace)
		AllocateIconManager("TWM", "Icons", "", 1);


		/*
		 * Mask over the screen with our welcome window stuff if we were
		 * asked to on the command line/environment; too early to get
		 * info from config file about it.
		 */
		screenmasked = false;
		if(dpy && takeover && Scr->ShowWelcomeWindow
		                && (welcomefile = getenv("CTWM_WELCOME_FILE"))) {
			screenmasked = true;
			MaskScreen(welcomefile);
		}



		/*
		 * Load up config file
		 */
		{
			bool ok = LoadTwmrc(CLarg.InitFile);

			// cfgchk just displays whether there are errors, then moves
			// on.
			if(CLarg.cfgchk) {
				if(ok) {
					fprintf(stderr, "%d: No errors found\n", scrnum);
				}
				else {
					fprintf(stderr, "%d: Errors found\n", scrnum);
					cfgerrs = true;
				}
				continue;
			}

			// In non-config-check mode, we historically proceed even if
			// there were errors, so keep doing that...
		}


		// For testing, it's useful to do all that initial setup up
		// through parsing, and then inspect Scr and the like.
		// Long-term, IWBNI we had a better way to do all the necessary
		// initialization and then call the parse ourselves at that
		// level.  But for now, provide a callback func that can pass
		// control back to the test code, then just exits.
		if(ctwm_test_postparse != NULL) {
			exit(ctwm_test_postparse());
		}



		/*
		 * Since we've loaded the config, go ahead and take over the
		 * screen.
		 */
		if(takeover) {
			if(takeover_screen(Scr) != true) {
				// Well, move on to the next one, maybe we'll get it...
				if(screenmasked) {
					UnmaskScreen();
				}
				continue;
			}

			// Well, we got this one
			numManaged++;
		}

		// If the config wants us to show the splash screen and we
		// haven't already, do it now.
		if(Scr->ShowWelcomeWindow && !screenmasked) {
			MaskScreen(NULL);
		}



		/*
		 * Do various setup based on the results from the config file.
		 */

		// Few simple var defaults
		if(Scr->ClickToFocus) {
			Scr->FocusRoot  = false;
			Scr->TitleFocus = false;
		}

		if(Scr->use3Dborders) {
			Scr->ClientBorderWidth = false;
		}


		// Now that we know what Border's there may be, create our
		// BorderedLayout.
		Scr->BorderedLayout = RLayoutCopyCropped(Scr->Layout,
		                      Scr->BorderLeft, Scr->BorderRight,
		                      Scr->BorderTop, Scr->BorderBottom);
		if(Scr->BorderedLayout == NULL) {
			Scr->BorderedLayout = Scr->Layout;        // nothing to crop
		}
		else if(Scr->BorderedLayout->monitors->len == 0) {
			fprintf(stderr,
			        "Borders too large! correct BorderLeft, BorderRight, BorderTop and/or BorderBottom parameters\n");
			exit(1);
		}
#ifdef DEBUG
		fprintf(stderr, "Bordered: ");
		RLayoutPrint(Scr->BorderedLayout);
#endif


		/*
		 * Setup stuff relating to VirtualScreens.  If something to do
		 * with it is set in the config, this all implements stuff needed
		 * for that.  If not, InitVirtualScreens() creates a single one
		 * mirroring our real root.
		 */
		InitVirtualScreens(Scr);
#ifdef VSCREEN
#ifdef EWMH
		EwmhInitVirtualRoots(Scr);
#endif /* EWMH */
#endif // vscreen

		// Setup WSM[s] (per-vscreen).  This also sets up the about the
		// workspaces for each vscreen and which is currently displayed.
		ConfigureWorkSpaceManager(Scr);


		/*
		 * Various decoration default overrides for 3d/2d.  Values that
		 * [presumtively] look "nice" on 75/100dpi displays.  -100 is a
		 * sentinel value we set before the config file parsing; since
		 * these defaults differ for 3d vs not, we can't just set them as
		 * default before the parse.
		 */
#define SETDEF(fld, num) if(Scr->fld == -100) { Scr->fld = num; }
		if(Scr->use3Dtitles) {
			SETDEF(FramePadding,  0);
			SETDEF(TitlePadding,  0);
			SETDEF(ButtonIndent,  0);
			SETDEF(TBInfo.border, 0);
		}
		else {
			SETDEF(FramePadding,  2);
			SETDEF(TitlePadding,  8);
			SETDEF(ButtonIndent,  1);
			SETDEF(TBInfo.border, 1);
		}
#undef SETDEF

		// These values are meaningless in !3d cases, so always zero them
		// out.
		if(! Scr->use3Dtitles) {
			Scr->TitleShadowDepth       = 0;
			Scr->TitleButtonShadowDepth = 0;
		}
		if(! Scr->use3Dborders) {
			Scr->BorderShadowDepth = 0;
		}
		if(! Scr->use3Dmenus) {
			Scr->MenuShadowDepth = 0;
		}
		if(! Scr->use3Diconmanagers) {
			Scr->IconManagerShadowDepth = 0;
		}
		if(! Scr->use3Dborders) {
			Scr->ThreeDBorderWidth = 0;
		}

		// Setup colors stuff
		if(!Scr->BeNiceToColormap) {
			// Default pair
			GetShadeColors(&Scr->DefaultC);

			// Various conditionally 3d bits
			if(Scr->use3Dtitles) {
				GetShadeColors(&Scr->TitleC);
			}
			if(Scr->use3Dmenus) {
				GetShadeColors(&Scr->MenuC);
			}
			if(Scr->use3Dmenus) {
				GetShadeColors(&Scr->MenuTitleC);
			}
			if(Scr->use3Dborders) {
				GetShadeColors(&Scr->BorderColorC);
			}
		}

		// Defaults for IconRegion bits that aren't set.
		for(IconRegion *ir = Scr->FirstRegion; ir; ir = ir->next) {
			if(ir->TitleJustification == TJ_UNDEF) {
				ir->TitleJustification = Scr->IconJustification;
			}
			if(ir->Justification == IRJ_UNDEF) {
				ir->Justification = Scr->IconRegionJustification;
			}
			if(ir->Alignement == IRA_UNDEF) {
				ir->Alignement = Scr->IconRegionAlignement;
			}
		}

		// Put the results of SaveColor{} into _MIT_PRIORITY_COLORS.
		assign_var_savecolor();

		// Setup cursor values that weren't give in the config
#define DEFCURSOR(name, val) if(!Scr->name) NewFontCursor(&Scr->name, val)
		DEFCURSOR(FrameCursor,   "top_left_arrow");
		DEFCURSOR(TitleCursor,   "top_left_arrow");
		DEFCURSOR(IconCursor,    "top_left_arrow");
		DEFCURSOR(IconMgrCursor, "top_left_arrow");
		DEFCURSOR(MoveCursor,    "fleur");
		DEFCURSOR(ResizeCursor,  "fleur");
		DEFCURSOR(MenuCursor,    "sb_left_arrow");
		DEFCURSOR(ButtonCursor,  "hand2");
		DEFCURSOR(WaitCursor,    "watch");
		DEFCURSOR(SelectCursor,  "dot");
		DEFCURSOR(DestroyCursor, "pirate");
		DEFCURSOR(AlterCursor,   "question_arrow");
#undef DEFCURSOR

		// Load up fonts for the screen.
		//
		// XXX HaveFonts is kinda stupid, however it gets useful in one
		// place: when loading button bindings, we make some sort of
		// "menu" for things (x-ref GotButton()), and the menu gen code
		// needs to load font stuff, so if that happened in the config
		// process, we would have already run CreateFonts().  Of course,
		// that's a order-dependent bit of the config file parsing too;
		// if you define the fonts too late, they wouldn't have been set
		// by then, and we won't [re]try them now...    arg.
		if(!Scr->HaveFonts) {
			CreateFonts(Scr);
		}

		// Adjust settings for titlebar.  Must follow CreateFonts() call
		// so we know these bits are populated
		Scr->TitleBarFont.y += Scr->FramePadding;
		Scr->TitleHeight = Scr->TitleBarFont.height + Scr->FramePadding * 2;
		if(Scr->use3Dtitles) {
			Scr->TitleHeight += 2 * Scr->TitleShadowDepth;
		}
		/* make title height be odd so buttons look nice and centered */
		if(!(Scr->TitleHeight & 1)) {
			Scr->TitleHeight++;
		}



		/*
		 * Now we can start making various things.
		 */

		// Stash up a ref to our Scr on the root, so we can find the
		// right Scr for events etc.
		XSaveContext(dpy, Scr->Root, ScreenContext, (XPointer) Scr);

		// Setup GC's for drawing, so we can start making stuff we have
		// to actually draw.  Could move earlier, has to preceed a lot of
		// following.
		CreateGCs();

		// Create and draw the menus we config'd
		MakeMenus();

		// Load up the images for titlebar buttons
		InitTitlebarButtons();

		// Allocate controls for WindowRegion's.  Has to follow
		// workspaces setup, but doesn't talk to X.
		CreateWindowRegions();

		// Copy the icon managers over to workspaces past the first as
		// necessary.  AllocateIconManager() and the config parsing
		// already made them on the first WS.
		AllocateOtherIconManagers();

		// Create the windows for our icon managers now that all our
		// tracking for it is setup.
		CreateIconManagers();

		// Create the WSM window (per-vscreen) and stash info on the root
		// about our WS's.
		CreateWorkSpaceManager();

		// Create the f.occupy window
		CreateOccupyWindow();

		// Setup TwmWorkspaces menu.  Needs workspaces setup, as well as
		// menus made.
		MakeWorkspacesMenu();

#ifdef WINBOX
		// setup WindowBox's
		createWindowBoxes();
#endif

		// Initialize Xrm stuff; things with setting occupation etc use
		// Xrm bits.
		XrmInitialize();

#ifdef EWMH
		// Set EWMH-related properties on various root-ish windows, for
		// other programs to read to find out how we view the world.
		EwmhInitScreenLate(Scr);
#endif /* EWMH */


		/*
		 * Look up and handle all the windows on the screen.
		 */
		{
			Window parent, *children;
			unsigned int nchildren;

			XQueryTree(dpy, Scr->Root, &croot, &parent, &children, &nchildren);

			/* Weed out icon windows */
			for(int i = 0; i < nchildren; i++) {
				if(children[i]) {
					XWMHints *wmhintsp = XGetWMHints(dpy, children[i]);

					if(wmhintsp) {
						if(wmhintsp->flags & IconWindowHint) {
							for(int j = 0; j < nchildren; j++) {
								if(children[j] == wmhintsp->icon_window) {
									children[j] = None;
									break;
								}
							}
						}
						XFree(wmhintsp);
					}
				}
			}

			/*
			 * Map all of the non-override windows.  This winds down
			 * into AddWindow() and friends through SimulateMapRequest(),
			 * so this is where we actually adopt the windows on the
			 * screen.
			 */
			for(int i = 0; i < nchildren; i++) {
				if(children[i] && MappedNotOverride(children[i])) {
					XUnmapWindow(dpy, children[i]);
					SimulateMapRequest(children[i]);
				}
			}

			/*
			 * At this point, we've adopted all the windows currently on
			 * the screen (aside from those we're intentionally not).
			 * Note that this happens _before_ the various other windows
			 * we create below, which is why they don't wind up getting
			 * TwmWindow's tied to them or show up in icon managers, etc.
			 * We'd need to actually make it _explicit_ that those
			 * windows aren't tracked by us if we changed that order...
			 */
		}


		// Show the WSM window if we should
		if(Scr->ShowWorkspaceManager && Scr->workSpaceManagerActive) {
			VirtualScreen *vs;
			if(Scr->WindowMask) {
				XRaiseWindow(dpy, Scr->WindowMask);
			}
			for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
				SetMapStateProp(vs->wsw->twm_win, NormalState);
				XMapWindow(dpy, vs->wsw->twm_win->frame);
				if(vs->wsw->twm_win->StartSqueezed) {
					Squeeze(vs->wsw->twm_win);
				}
				else {
					XMapWindow(dpy, vs->wsw->w);
				}
				vs->wsw->twm_win->mapped = true;
			}
		}


		/*
		 * Setup the Info window, used for f.identify and f.version.
		 */
		{
			unsigned long valuemask;
			XSetWindowAttributes attributes;

			attributes.border_pixel = Scr->DefaultC.fore;
			attributes.background_pixel = Scr->DefaultC.back;
			attributes.event_mask = (ExposureMask | ButtonPressMask |
			                         KeyPressMask | ButtonReleaseMask);
			NewFontCursor(&attributes.cursor, "hand2");
			valuemask = (CWBorderPixel | CWBackPixel | CWEventMask | CWCursor);
			Scr->InfoWindow.win =
			        XCreateWindow(dpy, Scr->Root, 0, 0,
			                      5, 5,
			                      0, 0,
			                      CopyFromParent, CopyFromParent,
			                      valuemask, &attributes);
		}


		/*
		 * Setup the Size/Position window for showing during resize/move
		 * operations.
		 */
		{
			// Stick the SizeWindow at the top left of the first monitor
			// we found on this Screen.  That _may_ not be (0,0) (imagine
			// a shorter left and taller right monitor, with their bottom
			// edges lined up instead of top), so we have to look up what
			// that coordinate is.  If we're CenterFeedbackWindow'ing,
			// the window will have to move between monitors depending on
			// where the window we're moving is (starts), but
			// MoveResizeSizeWindow() will handle that.  If not, it
			// always stays in the top-left of the first display.
			RArea area = RLayoutGetAreaIndex(Scr->Layout, 0);
			XRectangle ink_rect;
			XRectangle logical_rect;
			unsigned long valuemask;
			XSetWindowAttributes attributes;

			XmbTextExtents(Scr->SizeFont.font_set,
			               " 8888 x 8888 ", 13,
			               &ink_rect, &logical_rect);
			Scr->SizeStringWidth = logical_rect.width;
			valuemask = (CWBorderPixel | CWBackPixel | CWBitGravity);
			attributes.bit_gravity = NorthWestGravity;

			if(Scr->SaveUnder) {
				attributes.save_under = True;
				valuemask |= CWSaveUnder;
			}

			Scr->SizeWindow = XCreateWindow(dpy, Scr->Root,
			                                area.x, area.y,
			                                Scr->SizeStringWidth,
			                                (Scr->SizeFont.height +
			                                 SIZE_VINDENT * 2),
			                                0, 0,
			                                CopyFromParent,
			                                CopyFromParent,
			                                valuemask, &attributes);
		}

		// Create util window used in animation
		Scr->ShapeWindow = XCreateSimpleWindow(dpy, Scr->Root, 0, 0,
		                                       Scr->rootw, Scr->rooth, 0, 0, 0);


		// Clear out the splash screen if we had one
		if(Scr->ShowWelcomeWindow) {
			UnmaskScreen();
		}

		// Done setting up this Screen.  x-ref XXX's about whether this
		// element is worth anything...
		Scr->FirstTime = false;
	} // for each screen on display


	// If we're just checking the config, there's nothing more to do.
	if(CLarg.cfgchk) {
		exit(cfgerrs);
	}


	// We're not much of a window manager if we didn't get stuff to
	// manage...
	if(numManaged == 0) {
		if(CLarg.MultiScreen && NumScreens > 0)
			fprintf(stderr, "%s:  unable to find any unmanaged screens\n",
			        ProgramName);
		exit(1);
	}

#ifdef SESSION
	// Hook up session
	ConnectToSessionManager(CLarg.client_id);
#endif

#ifdef SOUNDS
	// Announce ourselves
	sound_load_list();
	play_startup_sound();
#endif

	// Hard-reset this flag.
	// XXX This doesn't seem right?
	RestartPreviousState = true;

	// Set vars to enable animation bits
	StartAnimation();

	// Main loop.
	HandlingEvents = true;
	HandleEvents();

	// Should never get here...
	fprintf(stderr, "Shouldn't return from HandleEvents()!\n");
	exit(1);
}



/**
 * Initialize ScreenInfo for a Screen.  This allocates the struct,
 * assigns in the info we pass it about the screen and dimensions, and
 * then puts in our various default/fallback/sentinel/etc values to
 * prepare it for later use.
 *
 * It is intentional that this doesn't do any of the initialization that
 * involves calling out to X functions; it operates as a pure function.
 * This makes it easier to use it to fake up a ScreenInfo for something
 * that isn't actually an X Screen, for testing etc.
 *
 * \param scrnum The Screen number (e.g, :0.0 -> 0)
 * \param croot  The X Window for the Screen's root window
 * \param crootx Root X coordinate
 * \param crooty Root Y coordinate
 * \param crootw Root width
 * \param crooth Root height
 * \return Allocated and populated ScreenInfo
 */
ScreenInfo *
InitScreenInfo(int scrnum, Window croot, int crootx, int crooty,
               unsigned int crootw, unsigned int crooth)
{
	ScreenInfo *scr;
	scr = calloc(1, sizeof(ScreenInfo));
	if(scr == NULL) {
		return NULL;
	}
	// Because of calloc(), it's already all 0 bytes, which are NULL and
	// false and 0 and similar.  Some following initializations are
	// nugatory because of that, but are left for clarity.

	// Poison the global Scr to protect against typos
#define Scr StupidProgrammer


	// Basic pieces about the X screen we're talking about, and some
	// derived dimension-related bits.
	scr->screen = scrnum;
	scr->XineramaRoot = scr->Root = croot;
	scr->rootx = crootx;
	scr->rooty = crooty;
	scr->rootw = crootw;
	scr->rooth = crooth;

#ifdef CAPTIVE
	scr->crootx = crootx;
	scr->crooty = crooty;
	scr->crootw = crootw;
	scr->crooth = crooth;
#endif

	// Don't allow icon titles wider than the screen
	scr->MaxIconTitleWidth = scr->rootw;

	// Attempt to come up with a sane default for the max sizes.  Start
	// by limiting so that a window with its left/top on the right/bottom
	// edge of the screen can't extend further than X can address (signed
	// 16-bit).  However, when your screen size starts approaching that
	// limit, reducing the max window sizes too much gets stupid too, so
	// set an arbitrary floor on how low this will take it.
	// MaxWindowSize in the config will override whatever's here anyway.
	scr->MaxWindowWidth  = 32767 - (scr->rootx + scr->rootw);
	scr->MaxWindowHeight = 32767 - (scr->rooty + scr->rooth);
	if(scr->MaxWindowWidth < 4096) {
		scr->MaxWindowWidth = 4096;
	}
	if(scr->MaxWindowHeight < 4096) {
		scr->MaxWindowHeight = 4096;
	}


	// Flags used in the code to keep track of where in various processes
	// (especially startup) we are.
	scr->HaveFonts = false;

	// Flag which basically means "initial screen setup time".
	// XXX Not clear to what extent this should even exist; a lot of
	// uses are fairly bogus.
	scr->FirstTime = true;

	// Sentinel values for defaulting config values
	scr->FramePadding = -100;
	scr->TitlePadding = -100;
	scr->ButtonIndent = -100;
	scr->TBInfo.border = -100;

	// Default values for all sorts of config params
	scr->SizeStringOffset = 0;
	scr->ThreeDBorderWidth = 6;
	scr->BorderWidth = BW;
	scr->IconBorderWidth = BW;
	scr->NumAutoRaises = 0;
	scr->NumAutoLowers = 0;
	scr->TransientOnTop = 30;
	scr->NoDefaults = false;
	scr->UsePPosition = PPOS_OFF;
	scr->UseSunkTitlePixmap = false;
	scr->FocusRoot = true;
	scr->WarpCursor = false;
	scr->ForceIcon = false;
	scr->NoGrabServer = true;
	scr->NoRaiseMove = false;
	scr->NoRaiseResize = false;
	scr->NoRaiseDeicon = false;
	scr->RaiseOnWarp = true;
	scr->DontMoveOff = false;
	scr->DoZoom = false;
	scr->TitleFocus = true;
	scr->IconManagerFocus = true;
	scr->StayUpMenus = false;
	scr->WarpToDefaultMenuEntry = false;
	scr->ClickToFocus = false;
	scr->SloppyFocus = false;
	scr->SaveWorkspaceFocus = false;
	scr->NoIconTitlebar = false;
	scr->NoTitlebar = false;
	scr->DecorateTransients = true;
	scr->IconifyByUnmapping = false;
	scr->ShowIconManager = false;
	scr->ShowWorkspaceManager = false;
	scr->WMgrButtonShadowDepth = 2;
	scr->WMgrVertButtonIndent  = 5;
	scr->WMgrHorizButtonIndent = 5;
	scr->BorderShadowDepth = 2;
	scr->TitleShadowDepth = 2;
	scr->TitleButtonShadowDepth = 2;
	scr->MenuShadowDepth = 2;
	scr->IconManagerShadowDepth = 2;
	scr->AutoOccupy = false;
	scr->TransientHasOccupation = false;
	scr->DontPaintRootWindow = false;
	scr->IconManagerDontShow = false;
	scr->BackingStore = false;
	scr->SaveUnder = true;
	scr->RandomPlacement = RP_ALL;
	scr->RandomDisplacementX = 30;
	scr->RandomDisplacementY = 30;
	scr->DoOpaqueMove = true;
	scr->OpaqueMove = false;
	scr->OpaqueMoveThreshold = 200;
	scr->OpaqueResize = false;
	scr->DoOpaqueResize = true;
	scr->OpaqueResizeThreshold = 1000;
	scr->Highlight = true;
	scr->StackMode = true;
	scr->TitleHighlight = true;
	scr->MoveDelta = 1;
	scr->MoveOffResistance = -1;
	scr->MovePackResistance = 20;
	scr->ZoomCount = 8;
	scr->SortIconMgr = true;
	scr->Shadow = true;
	scr->InterpolateMenuColors = false;
	scr->NoIconManagers = false;
	scr->ClientBorderWidth = false;
	scr->SqueezeTitle = false;
	scr->FirstTime = true;
	scr->CaseSensitive = true;
	scr->WarpUnmapped = false;
	scr->WindowRingAll = false;
	scr->WarpRingAnyWhere = true;
	scr->ShortAllWindowsMenus = false;
	scr->use3Diconmanagers = false;
	scr->use3Dmenus = false;
	scr->use3Dtitles = false;
	scr->use3Dborders = false;
	scr->use3Dwmap = false;
	scr->SunkFocusWindowTitle = false;
	scr->ClearShadowContrast = 50;
	scr->DarkShadowContrast  = 40;
	scr->BeNiceToColormap = false;
	scr->BorderCursors = false;
	scr->IconJustification = TJ_CENTER;
	scr->IconRegionJustification = IRJ_CENTER;
	scr->IconRegionAlignement = IRA_CENTER;
	scr->TitleJustification = TJ_LEFT;
	scr->IconifyStyle = ICONIFY_NORMAL;
	scr->ReallyMoveInWorkspaceManager = false;
	scr->ShowWinWhenMovingInWmgr = false;
	scr->ReverseCurrentWorkspace = false;
	scr->DontWarpCursorInWMap = false;
	scr->XMoveGrid = 1;
	scr->YMoveGrid = 1;
	scr->CenterFeedbackWindow = false;
	scr->ShrinkIconTitles = false;
	scr->AutoRaiseIcons = false;
	scr->AutoFocusToTransients = false;
	scr->OpenWindowTimeout = 0;
	scr->RaiseWhenAutoUnSqueeze = false;
	scr->RaiseOnClick = false;
	scr->RaiseOnClickButton = 1;
	scr->IgnoreModifier = 0;
	scr->IgnoreCaseInMenuSelection = false;
	scr->PackNewWindows = false;
	scr->AlwaysSqueezeToGravity = false;
	scr->NoWarpToMenuTitle = false;
	scr->DontToggleWorkspaceManagerState = false;
	scr->NameDecorations = true;
	scr->ForceFocus = false;
	scr->BorderTop    = 0;
	scr->BorderBottom = 0;
	scr->BorderLeft   = 0;
	scr->BorderRight  = 0;
	scr->PixmapDirectory   = PIXMAP_DIRECTORY;
#ifdef EWMH
	scr->PreferredIconWidth = 48;
	scr->PreferredIconHeight = 48;

	scr->ewmh_CLIENT_LIST_used = 0;
	scr->ewmh_CLIENT_LIST_size = 16;
	scr->ewmh_CLIENT_LIST = calloc(scr->ewmh_CLIENT_LIST_size,
	                               sizeof(scr->ewmh_CLIENT_LIST[0]));
	if(scr->ewmh_CLIENT_LIST == NULL) {
		free(scr);
		return NULL;
	}
#endif

	// OTP structure bits
	OtpScrInitData(scr);


	// WorkSpaceManager stuff
	scr->workSpaceMgr.initialstate  = WMS_map;
	scr->workSpaceMgr.buttonStyle   = STYLE_NORMAL;
	scr->workSpaceMgr.vspace        = scr->WMgrVertButtonIndent;
	scr->workSpaceMgr.hspace        = scr->WMgrHorizButtonIndent;

	scr->workSpaceMgr.occupyWindow = calloc(1, sizeof(OccupyWindow));
	scr->workSpaceMgr.occupyWindow->vspace    = scr->WMgrVertButtonIndent;
	scr->workSpaceMgr.occupyWindow->hspace    = scr->WMgrHorizButtonIndent;
	scr->workSpaceMgr.occupyWindow->name      = "Occupy Window";
	scr->workSpaceMgr.occupyWindow->icon_name = "Occupy Window Icon";

	scr->workSpaceMgr.name      = "WorkSpaceManager";
	scr->workSpaceMgr.icon_name = "WorkSpaceManager Icon";


	// Setup default fonts in case the config file doesn't
#define DEFAULT_NICE_FONT "-*-helvetica-bold-r-normal-*-*-120-*"
#define DEFAULT_FAST_FONT "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-*"
#define SETFONT(fld, var) (scr->fld##Font.basename = DEFAULT_##var##_FONT)

	SETFONT(TitleBar,    NICE);
	SETFONT(Menu,        NICE);
	SETFONT(Icon,        NICE);
	SETFONT(Size,        FAST);
	SETFONT(IconManager, NICE);
	SETFONT(Default,     FAST);
	scr->workSpaceMgr.windowFont.basename =
	        "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1";

#undef SETFONT
#undef DEFAULT_FAST_FONT
#undef DEFAULT_NICE_FONT


	// Set some fallback values that we set from the X server, for
	// special cases where we may not actually be talking to one.
	scr->d_depth = 24;
	scr->RealRoot = croot;
	scr->mm_w = 406; // 16 in
	scr->mm_h = 229; // 9 in
	scr->Monochrome = COLOR;

	// Cleanup poisoning
#undef Scr
	return scr;
}




#ifdef CAPTIVE
/**
 * Create a new window to use for a captive ctwm.
 */
static Window
CreateCaptiveRootWindow(int x, int y,
                        unsigned int width, unsigned int height)
{
	int         scrnum;
	Window      ret;
	XWMHints    wmhints;

	scrnum = DefaultScreen(dpy);
	ret = XCreateSimpleWindow(dpy, RootWindow(dpy, scrnum),
	                          x, y, width, height, 2, WhitePixel(dpy, scrnum),
	                          BlackPixel(dpy, scrnum));
	wmhints.initial_state = NormalState;
	wmhints.input         = True;
	wmhints.flags         = InputHint | StateHint;

	XmbSetWMProperties(dpy, ret, "Captive ctwm", NULL, NULL, 0, NULL,
	                   &wmhints, NULL);
	XChangeProperty(dpy, ret, XA_WM_CTWM_ROOT, XA_WINDOW, 32,
	                PropModeReplace, (unsigned char *) &ret, 1);
	XSelectInput(dpy, ret, StructureNotifyMask);
	XMapWindow(dpy, ret);
	return (ret);
}
#endif



/**
 * Return true if a window is not set to override_redirect ("Hey!  WM!
 * Leave those wins alone!"), and isn't unmapped.  Used during startup to
 * fake mapping for wins that should be up.
 */
static bool
MappedNotOverride(Window w)
{
	XWindowAttributes wa;

	XGetWindowAttributes(dpy, w, &wa);
	return ((wa.map_state != IsUnmapped) && (wa.override_redirect != True));
}
