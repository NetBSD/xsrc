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
#include <signal.h>
#include <unistd.h>
#include <locale.h>

#ifdef __WAIT_FOR_CHILDS
#  include <sys/wait.h>
#endif

#include <fcntl.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Error.h>
#include <X11/extensions/shape.h>


#include "ctwm_atoms.h"
#include "ctwm_main.h"
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
#include "session.h"
#include "occupation.h"
#include "otp.h"
#include "cursor.h"
#include "windowbox.h"
#include "captive.h"
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

int NumScreens;                 /* number of screens in ScreenList */
bool HasShape;                  /* server supports shape extension? */
int ShapeEventBase, ShapeErrorBase;
ScreenInfo **ScreenList;        /* structures for each screen */
ScreenInfo *Scr = NULL;         /* the cur and prev screens */
int PreviousScreen;             /* last screen that we were on */
static bool RedirectError;      /* true ==> another window manager running */
/* for settting RedirectError */
static int CatchRedirectError(Display *display, XErrorEvent *event);
/* for everything else */
static int TwmErrorHandler(Display *display, XErrorEvent *event);
static Window CreateCaptiveRootWindow(int x, int y,
                                      unsigned int width, unsigned int height);
static void InternUsefulAtoms(void);
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
int Argc;
char **Argv;

bool RestartPreviousState = true;      /* try to restart in previous state */

bool RestartFlag = false;
SIGNAL_T Restart(int signum);
SIGNAL_T Crash(int signum);
#ifdef __WAIT_FOR_CHILDS
SIGNAL_T ChildExit(int signum);
#endif

/***********************************************************************
 *
 *  Procedure:
 *      main - start of twm
 *
 ***********************************************************************
 */

int
ctwm_main(int argc, char *argv[])
{
	int numManaged, firstscrn, lastscrn;
	bool FirstScreen;

	setlocale(LC_ALL, "");

	ProgramName = argv[0];
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


#define newhandler(sig, action) \
    if (signal (sig, SIG_IGN) != SIG_IGN) signal (sig, action)

	newhandler(SIGINT, Done);
	signal(SIGHUP, Restart);
	newhandler(SIGQUIT, Done);
	newhandler(SIGTERM, Done);
#ifdef __WAIT_FOR_CHILDS
	newhandler(SIGCHLD, ChildExit);
#endif
	signal(SIGALRM, SIG_IGN);
#ifdef NOTRAP
	signal(SIGSEGV, Crash);
	signal(SIGBUS,  Crash);
#endif

#undef newhandler

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

		if(!(dpy = XtOpenDisplay(appContext, CLarg.display_name, "twm", "twm",
		                         NULL, 0, &zero, NULL))) {
			fprintf(stderr, "%s:  unable to open display \"%s\"\n",
			        ProgramName, XDisplayName(CLarg.display_name));
			exit(1);
		}

		if(fcntl(ConnectionNumber(dpy), F_SETFD, FD_CLOEXEC) == -1) {
			fprintf(stderr,
			        "%s:  unable to mark display connection as close-on-exec\n",
			        ProgramName);
			exit(1);
		}
	}


	// Load session stuff
	if(CLarg.restore_filename) {
		ReadWinConfigFile(CLarg.restore_filename);
	}

	// Load up info about X extensions
	HasShape = XShapeQueryExtension(dpy, &ShapeEventBase, &ShapeErrorBase);

	// Allocate contexts/atoms/etc we use
	TwmContext = XUniqueContext();
	MenuContext = XUniqueContext();
	ScreenContext = XUniqueContext();
	ColormapContext = XUniqueContext();
	InitWorkSpaceManagerContext();

	InternUsefulAtoms();

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
	NumScreens = ScreenCount(dpy);
	if(CLarg.MultiScreen) {
		firstscrn = 0;
		lastscrn = NumScreens - 1;
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

	// Initialize
	PreviousScreen = DefaultScreen(dpy);


	// Do a little early initialization
#ifdef EWMH
	EwmhInit();
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
		unsigned long attrmask;
		int crootx, crooty;
		unsigned int crootw, crooth;
		bool screenmasked;
		char *welcomefile;

		/*
		 * First, setup the root window for the screen.
		 */
		if(CLarg.is_captive) {
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
		else {
			// Normal; get the real display's root.
			croot  = RootWindow(dpy, scrnum);
			crootx = 0;
			crooty = 0;
			crootw = DisplayWidth(dpy, scrnum);
			crooth = DisplayHeight(dpy, scrnum);
		}

		// Initialize to empty.  This gets populated with SaveColor{}
		// results.  String values get done via assign_var_savecolor()
		// call below, but keyword choicse wind up getting put in on the
		// fly during config file parsing, so we have to clear it before
		// we get to the config.
		// XXX Maybe we should change that...
		XChangeProperty(dpy, croot, XA__MIT_PRIORITY_COLORS,
		                XA_CARDINAL, 32, PropModeReplace, NULL, 0);


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

		// Not trying to take over if we're just checking config or
		// making a new captive ctwm.
		if(CLarg.cfgchk || CLarg.is_captive) {
			Scr->takeover = false;
		}

		// Other misc adjustments to default config.
		Scr->ShowWelcomeWindow = CLarg.ShowWelcomeWindow;


#ifdef EWMH
		// Early EWMH setup
		EwmhInitScreenEarly(Scr);
#endif /* EWMH */

		// Early OTP setup
		OtpScrInitData(Scr);


		/*
		 * Subscribe to various events on the root window.  Because X
		 * only allows a single client to subscribe to
		 * SubstructureRedirect and ButtonPress bits, this also serves to
		 * mutex who is The WM for the root window, and thus (aside from
		 * captive) the Screen.
		 *
		 * To catch whether that failed, we set a special one-shot error
		 * handler to flip a var that we test to find out whether the
		 * redirect failed.
		 */
		XSync(dpy, 0); // Flush possible previous errors
		RedirectError = false;
		XSetErrorHandler(CatchRedirectError);
		attrmask = ColormapChangeMask | EnterWindowMask | PropertyChangeMask |
		           SubstructureRedirectMask | KeyPressMask | ButtonPressMask |
		           ButtonReleaseMask;
#ifdef EWMH
		attrmask |= StructureNotifyMask;
#endif /* EWMH */
		if(CLarg.is_captive) {
			attrmask |= StructureNotifyMask;
		}
		XSelectInput(dpy, croot, attrmask);
		XSync(dpy, 0); // Flush the RedirectError, if we had one

		// Back to our normal handler
		XSetErrorHandler(TwmErrorHandler);

		if(RedirectError && Scr->takeover) {
			fprintf(stderr, "%s:  another window manager is already running",
			        ProgramName);
			if(CLarg.MultiScreen && NumScreens > 0) {
				fprintf(stderr, " on screen %d?\n", scrnum);
			}
			else {
				fprintf(stderr, "?\n");
			}

			// XSetErrorHandler() isn't local to the Screen; it's for the
			// whole connection.  We wind up in a slightly weird state
			// once we've set it up, but decided we aren't taking over
			// this screen, but resetting it would be a little weird too,
			// because maybe we have taken over some other screen.  So,
			// just throw up our hands.
			continue;
		}


		// We now manage it (or are in the various special circumstances
		// where it's near enough).
		numManaged ++;


		// Now we can stash some info about the screen
		Scr->d_depth = DefaultDepth(dpy, scrnum);
		Scr->d_visual = DefaultVisual(dpy, scrnum);
		Scr->RealRoot = RootWindow(dpy, scrnum);

		// Now that we have d_depth...
		Scr->XORvalue = (((unsigned long) 1) << Scr->d_depth) - 1;

		// Stash up a ref to our Scr on the root, so we can find the
		// right Scr for events etc.
		XSaveContext(dpy, Scr->Root, ScreenContext, (XPointer) Scr);

		// Init captive bits
		if(CLarg.is_captive) {
			Scr->CaptiveRoot = croot;
			Scr->captivename = AddToCaptiveList(CLarg.captivename);
			if(Scr->captivename) {
				XmbSetWMProperties(dpy, croot,
				                   Scr->captivename, Scr->captivename,
				                   NULL, 0, NULL, NULL, NULL);
			}
		}


		// Init some colormap bits
		{
			// 1 on the root
			Scr->RootColormaps.number_cwins = 1;
			Scr->RootColormaps.cwins = malloc(sizeof(ColormapWindow *));
			Scr->RootColormaps.cwins[0] = CreateColormapWindow(Scr->Root, true,
			                              false);
			Scr->RootColormaps.cwins[0]->visibility = VisibilityPartiallyObscured;

			// Initialize storage for all maps the Screen can hold
			Scr->cmapInfo.cmaps = NULL;
			Scr->cmapInfo.maxCmaps = MaxCmapsOfScreen(ScreenOfDisplay(dpy, Scr->screen));
			Scr->cmapInfo.root_pushes = 0;
			InstallColormaps(0, &Scr->RootColormaps);

			// Setup which we're using
			Scr->StdCmapInfo.head = Scr->StdCmapInfo.tail
			                        = Scr->StdCmapInfo.mru = NULL;
			Scr->StdCmapInfo.mruindex = 0;
			LocateStandardColormaps();
		}


		// Are we monochrome?  Or do we care this millennium?
		if(CLarg.Monochrome || DisplayCells(dpy, scrnum) < 3) {
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
		if(FirstScreen) {
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
		if(Scr->ShowWelcomeWindow && (welcomefile = getenv("CTWM_WELCOME_FILE"))) {
			screenmasked = true;
			MaskScreen(welcomefile);
		}


		/*
		 * Load up config file
		 */
		if(CLarg.cfgchk) {
			if(LoadTwmrc(CLarg.InitFile) == false) {
				/* Error return */
				fprintf(stderr, "Errors found\n");
				exit(1);
			}
			else {
				fprintf(stderr, "No errors found\n");
				exit(0);
			}
		}
		else {
			LoadTwmrc(CLarg.InitFile);
		}


		/*
		 * Setup stuff relating to VirtualScreens.  If something to do
		 * with it is set in the config, this all implements stuff needed
		 * for that.  If not, InitVirtualScreens() creates a single one
		 * mirroring our real root.
		 */
		InitVirtualScreens(Scr);
#ifdef EWMH
		EwmhInitVirtualRoots(Scr);
#endif /* EWMH */

		// Setup WSM[s] (per-vscreen)
		ConfigureWorkSpaceManager();

		// If the config wants us to show the splash screen and we
		// haven't already, do it now.
		if(Scr->ShowWelcomeWindow && !screenmasked) {
			MaskScreen(NULL);
		}



		/*
		 * Do various setup based on the results from the config file.
		 */
		if(Scr->ClickToFocus) {
			Scr->FocusRoot  = false;
			Scr->TitleFocus = false;
		}

		if(Scr->use3Dborders) {
			Scr->ClientBorderWidth = false;
		}


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

		// setup WindowBox's
		createWindowBoxes();

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
			int sx, sy;
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

			if(Scr->CenterFeedbackWindow) {
				sx = (Scr->rootw / 2) - (Scr->SizeStringWidth / 2);
				sy = (Scr->rooth / 2) - ((Scr->SizeFont.height + SIZE_VINDENT * 2) / 2);
				if(Scr->SaveUnder) {
					attributes.save_under = True;
					valuemask |= CWSaveUnder;
				}
			}
			else {
				sx = 0;
				sy = 0;
			}
			Scr->SizeWindow = XCreateWindow(dpy, Scr->Root, sx, sy,
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


	// We're not much of a window manager if we didn't get stuff to
	// manage...
	if(numManaged == 0) {
		if(CLarg.MultiScreen && NumScreens > 0)
			fprintf(stderr, "%s:  unable to find any unmanaged screens\n",
			        ProgramName);
		exit(1);
	}

	// Hook up session
	ConnectToSessionManager(CLarg.client_id);

#ifdef SOUNDS
	// Announce ourselves
	sound_load_list();
	play_startup_sound();
#endif

	// Hard-reset this flag.
	// XXX This doesn't seem right?
	RestartPreviousState = true;

	// Do some late initialization
	HandlingEvents = true;
	StartAnimation();

	// Main loop.
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
	scr->rootx = scr->crootx = crootx;
	scr->rooty = scr->crooty = crooty;
	scr->rootw = scr->crootw = crootw;
	scr->rooth = scr->crooth = crooth;

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

	// We're a WM, we're usually trying to take over (x-ref later code in
	// caller)
	scr->takeover = true;

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
#endif


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

	// Cleanup poisoning
#undef Scr
	return scr;
}


void CreateFonts(ScreenInfo *scr)
{
#define LOADFONT(fld) (GetFont(&scr->fld##Font))
	LOADFONT(TitleBar);
	LOADFONT(Menu);
	LOADFONT(Icon);
	LOADFONT(Size);
	LOADFONT(IconManager);
	LOADFONT(Default);
	LOADFONT(workSpaceMgr.window);
#undef LOADFONT

	scr->HaveFonts = true;
}


void RestoreWithdrawnLocation(TwmWindow *tmp)
{
	int gravx, gravy;
	unsigned int bw, mask;
	XWindowChanges xwc;

	if(tmp->UnmapByMovingFarAway && !visible(tmp)) {
		XMoveWindow(dpy, tmp->frame, tmp->frame_x, tmp->frame_y);
	}
	if(tmp->squeezed) {
		Squeeze(tmp);
	}
	if(XGetGeometry(dpy, tmp->w, &JunkRoot, &xwc.x, &xwc.y,
	                &JunkWidth, &JunkHeight, &bw, &JunkDepth)) {

		GetGravityOffsets(tmp, &gravx, &gravy);
		if(gravy < 0) {
			xwc.y -= tmp->title_height;
		}
		xwc.x += gravx * tmp->frame_bw3D;
		xwc.y += gravy * tmp->frame_bw3D;

		if(bw != tmp->old_bw) {
			int xoff, yoff;

			if(!Scr->ClientBorderWidth) {
				xoff = gravx;
				yoff = gravy;
			}
			else {
				xoff = 0;
				yoff = 0;
			}

			xwc.x -= (xoff + 1) * tmp->old_bw;
			xwc.y -= (yoff + 1) * tmp->old_bw;
		}
		if(!Scr->ClientBorderWidth) {
			xwc.x += gravx * tmp->frame_bw;
			xwc.y += gravy * tmp->frame_bw;
		}

		mask = (CWX | CWY);
		if(bw != tmp->old_bw) {
			xwc.border_width = tmp->old_bw;
			mask |= CWBorderWidth;
		}

#if 0
		if(tmp->vs) {
			xwc.x += tmp->vs->x;
			xwc.y += tmp->vs->y;
		}
#endif

		if(tmp->winbox && tmp->winbox->twmwin && tmp->frame) {
			int xbox, ybox;
			if(XGetGeometry(dpy, tmp->frame, &JunkRoot, &xbox, &ybox,
			                &JunkWidth, &JunkHeight, &bw, &JunkDepth)) {
				ReparentWindow(dpy, tmp, WinWin, Scr->Root, xbox, ybox);
			}
		}
		XConfigureWindow(dpy, tmp->w, mask, &xwc);

		if(tmp->wmhints->flags & IconWindowHint) {
			XUnmapWindow(dpy, tmp->wmhints->icon_window);
		}

	}
}


/***********************************************************************
 *
 *  Procedure:
 *      Done - cleanup and exit twm
 *
 *  Returned Value:
 *      none
 *
 *  Inputs:
 *      none
 *
 *  Outputs:
 *      none
 *
 *  Special Considerations:
 *      none
 *
 ***********************************************************************
 */

void Reborder(Time mytime)
{
	TwmWindow *tmp;                     /* temp twm window structure */
	int scrnum;
	ScreenInfo *savedScreen;            /* Its better to avoid coredumps */

	/* put a border back around all windows */

	XGrabServer(dpy);
	savedScreen = Scr;
	for(scrnum = 0; scrnum < NumScreens; scrnum++) {
		if((Scr = ScreenList[scrnum]) == NULL) {
			continue;
		}

		InstallColormaps(0, &Scr->RootColormaps);       /* force reinstall */
		for(tmp = Scr->FirstWindow; tmp != NULL; tmp = tmp->next) {
			RestoreWithdrawnLocation(tmp);
			XMapWindow(dpy, tmp->w);
		}
	}
	Scr = savedScreen;
	XUngrabServer(dpy);
	SetFocus(NULL, mytime);
}

SIGNAL_T Done(int signum)
{
#ifdef SOUNDS
	play_exit_sound();
#endif
	Reborder(CurrentTime);
#ifdef EWMH
	EwmhTerminate();
#endif /* EWMH */
	XDeleteProperty(dpy, Scr->Root, XA_WM_WORKSPACESLIST);
	if(CLarg.is_captive) {
		RemoveFromCaptiveList(Scr->captivename);
	}
	XCloseDisplay(dpy);
	exit(0);
}

SIGNAL_T Crash(int signum)
{
	Reborder(CurrentTime);
	XDeleteProperty(dpy, Scr->Root, XA_WM_WORKSPACESLIST);
	if(CLarg.is_captive) {
		RemoveFromCaptiveList(Scr->captivename);
	}
	XCloseDisplay(dpy);

	fprintf(stderr, "\nCongratulations, you have found a bug in ctwm\n");
	fprintf(stderr, "If a core file was generated in your directory,\n");
	fprintf(stderr, "can you please try extract the stack trace,\n");
	fprintf(stderr,
	        "and mail the results, and a description of what you were doing,\n");
	fprintf(stderr, "to ctwm@ctwm.org.  Thank you for your support.\n");
	fprintf(stderr, "...exiting ctwm now.\n\n");

	abort();
}


SIGNAL_T Restart(int signum)
{
	fprintf(stderr, "%s:  setting restart flag\n", ProgramName);
	RestartFlag = true;
}

void DoRestart(Time t)
{
	RestartFlag = false;

	StopAnimation();
	XSync(dpy, 0);
	Reborder(t);
	XSync(dpy, 0);

	if(smcConn) {
		SmcCloseConnection(smcConn, 0, NULL);
	}

	fprintf(stderr, "%s:  restarting:  %s\n",
	        ProgramName, *Argv);
	execvp(*Argv, Argv);
	fprintf(stderr, "%s:  unable to restart:  %s\n", ProgramName, *Argv);
}

#ifdef __WAIT_FOR_CHILDS
/*
 * Handler for SIGCHLD. Needed to avoid zombies when an .xinitrc
 * execs ctwm as the last client. (All processes forked off from
 * within .xinitrc have been inherited by ctwm during the exec.)
 * Jens Schweikhardt <jens@kssun3.rus.uni-stuttgart.de>
 */
SIGNAL_T
ChildExit(int signum)
{
	int Errno = errno;
	signal(SIGCHLD, ChildExit);  /* reestablish because we're a one-shot */
	waitpid(-1, NULL, WNOHANG);   /* reap dead child, ignore status */
	errno = Errno;               /* restore errno for interrupted sys calls */
}
#endif

/*
 * Error Handlers.  If a client dies, we'll get a BadWindow error (except for
 * GetGeometry which returns BadDrawable) for most operations that we do before
 * manipulating the client's window.
 */

static XErrorEvent LastErrorEvent;

static int TwmErrorHandler(Display *display, XErrorEvent *event)
{
	LastErrorEvent = *event;

	if(CLarg.PrintErrorMessages &&                 /* don't be too obnoxious */
	                event->error_code != BadWindow &&       /* watch for dead puppies */
	                (event->request_code != X_GetGeometry &&         /* of all styles */
	                 event->error_code != BadDrawable)) {
		XmuPrintDefaultErrorMessage(display, event, stderr);
	}
	return 0;
}


/* ARGSUSED*/
static int CatchRedirectError(Display *display, XErrorEvent *event)
{
	RedirectError = true;
	LastErrorEvent = *event;
	return 0;
}

/*
 * XA_MIT_PRIORITY_COLORS     Create priority colors if necessary.
 * XA_WM_END_OF_ANIMATION     Used to throttle animation.
 */

Atom XCTWMAtom[NUM_CTWM_XATOMS];

void InternUsefulAtoms(void)
{
	XInternAtoms(dpy, XCTWMAtomNames, NUM_CTWM_XATOMS, False, XCTWMAtom);
}

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


/*
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
