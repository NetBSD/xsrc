/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: xtest.h,v 1.38 94/04/17 21:00:05 rws Exp $
 */

#define TEST_ANSI (__STDC__ == 1)

/* 
 * Define NULL if not already done - really just SunOS writearound.
 * NULL should be in stdlib.h - but SunOS4.1 does not have it.
 */
#ifndef	NULL
#define NULL	0
#endif

#define ALLEVENTS  \
(KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask| \
EnterWindowMask|LeaveWindowMask|PointerMotionMask|PointerMotionHintMask| \
Button1MotionMask|Button2MotionMask|Button3MotionMask|Button4MotionMask| \
Button5MotionMask|ButtonMotionMask|KeymapStateMask|ExposureMask| \
VisibilityChangeMask|StructureNotifyMask|ResizeRedirectMask| \
SubstructureNotifyMask|SubstructureRedirectMask|FocusChangeMask| \
PropertyChangeMask|ColormapChangeMask|OwnerGrabButtonMask)

struct	linkinfo	{
	char	*name;		/* name of executable link */
	char	*testname;	/* Actual name of function */
	int 	*ntests;	/* Number of TP's for this Test Case */
	struct	tet_testlist *testlist; /* test purpose list */
	void	(*localstartup)();	/* A local startup routine */
	void	(*localcleanup)();	/* A local cleanup routine */
};

/*
 * Macros to use when not using XCALL
 */
#define	BASIC_STARTCALL(d)	\
	startcall(d);\
	if (isdeleted())\
		return
#define	BASIC_ENDCALL(d, expected)	\
	do {\
		endcall(d);\
		if (geterr() != expected) {\
			report("Got %s, Expecting %s", errorname(geterr()), errorname(expected));\
			FAIL;\
		}\
	} while (0)


/*
 * Invocable component types
 */
#define	Good	1
#define Bad 	2

/*
 * Standard parameters for windows and pixmaps.
 */
#define	W_STDWIDTH	100
#define W_STDHEIGHT	90
#define W_BG	0L	/* Background pixel */
#define W_FG	1L	/* Forground pixel */

/* Size for images */
#define	I_STDWIDTH	100
#define I_STDHEIGHT	90

/* A shorthand for DefaultRootWindow */
#define DRW	DefaultRootWindow

/* Parameters used for font paths */
#define	MAX_DIRS	32
#define	SEP	","

/*
 * MIT specific test results codes.
 */
#define	MIT_TET_WARNING	101
#define	MIT_TET_FIP	102
#define	MIT_TET_ABORT	103

/*
 * Path check macros
 */
#define	CHECK	do {\
		++pass; \
		check("%s-%d  %d, line %d", TestName, tet_thistest, pass, __LINE__); \
		} while (0)

#define PASS	tet_result(TET_PASS)
#define FAIL    do { fail++; if (!isdeleted()) tet_result(TET_FAIL); } while (0)
#define UNTESTED	tet_result(TET_UNTESTED)
#define UNSUPPORTED	tet_result(TET_UNSUPPORTED)
#define UNRESOLVED	tet_result(TET_UNRESOLVED)
#define NOTINUSE	tet_result(TET_NOTINUSE)
#define WARNING		tet_result(MIT_TET_WARNING)
#define FIP		tet_result(MIT_TET_FIP)
#define	ABORT		tet_result(MIT_TET_ABORT)
#define	CHECKPASS(n) \
do { \
	if (n && n == pass && fail == 0) \
		PASS; \
	else if (fail == 0) {\
		if (n == 0) \
			report("No CHECK marks encountered"); \
		else \
			report("Path check error (%d should be %d)", pass, n);\
		report("This is usually caused by a programming error in the test-suite"); \
		UNRESOLVED;\
	} \
} while (0)
#define	CHECKUNTESTED(n) \
do { \
	if (n && n == pass && fail == 0) \
		untested("The assertion can only be partially tested"); \
	else if (fail == 0) {\
		if (n == 0) \
			report("Path check error - no CHECK marks encountered"); \
		else \
			report("Path check error (%d should be %d)", pass, n);\
		report("This is usually caused by a programming error in the test-suite"); \
		UNRESOLVED;\
	} \
} while (0)
#define	CHECKFIP(n) \
do { \
	if (n && n == pass && fail == 0) \
		FIP; \
	else if (fail == 0) {\
		if (n == 0) \
			report("Path check error - no CHECK marks encountered"); \
		else \
			report("Path check error (%d should be %d)", pass, n);\
		report("This is usually caused by a programming error in the test-suite"); \
		UNRESOLVED;\
	} \
} while (0)


/*
 * A macro to do the often repeated check, pass/fail operation
 */
#define	PIXCHECK(DSP, D)	\
	do { \
		if (verifyimage(DSP, D, (struct area *)0)) \
			CHECK; \
		else \
			FAIL; \
	} while (0)

/*
 * Get the number of elements in an array.  Must be passed a real array
 * only.
 */
#define NELEM(array) (sizeof(array)/sizeof(array[0]))

/*
 * Create a plane mask for a depth.
 */
#define	DEPTHMASK(D) (((D)==32)? 0xffffffff: (1<<(D))-1)

/*
 * Set all the members of an area structure in one go.
 */
#define	setarea(_A, _X, _Y, _Width, _Height) \
	do { \
		(_A)->x = _X; \
		(_A)->y = _Y; \
		(_A)->width = _Width; \
		(_A)->height = _Height; \
	} while (0);

/* Number of fonts used in xtest */
#define	XT_NFONTS	7

/* Name of a propoerty used by gettime() and some of the CH08 tests */
#define XT_TIMESTAMP	"XTEST_TIMESTAMP"

/*
 * Options are all set into this structure.
 */
struct	config	{
	char	*display;		/* The display string */
	int 	alt_screen;		/* Alternate screen number */
	int 	fontcursor_good;	/* A good value in the cursor font */
	int 	fontcursor_bad;		/* A bad value in the cursor font */
	char	*fontdir;		/* Font location for pixel generation */
	char	*fontpath_good;		/* known good path for fonts */
	char	*fontpath_bad;		/* known bad path for fonts */
	char	*bad_font_name;		/* known bad font name */
	int 	save_server_image;	/* Save server images */
	char	*good_colorname;	/* known good name */
	char	*bad_colorname;		/* known bad name */
	int 	option_no_check;	/* No check messages in the journal */
	int 	option_no_trace;	/* No trace messages in the journal */
	int 	debug;			/* debug level */
	int 	debug_override_redirect;/* Use override redirect on windows */
	int 	debug_pause_after;	/* pause after each XCALL */
	int 	debug_pixmap_only;	/* use only pixmaps */
	int 	debug_window_only;	/* use only windows */
	int 	debug_default_depths;	/* use default depth/visual */
	int	speedfactor;		/* used as multiplier when timing */
	int	displaymotionbuffersize;/* value to be returned by ... */
	char	*fontpath;		/* font path for test fonts */
	int	posix_system;		/* whether posix system */
	int	protocol_version;	/* protocol version */
	int	protocol_revision;	/* protocol revision */
	int	vendor_release;		/* vendor release */
	int	does_save_unders;	/* save unders supported */
	int	does_backing_store;	/* backing store supported */
	int	decnet;			/* decnet supported */
	int	tcp;			/* tcp supported */
	char	*displayhost;		/* hostname for XOpenDisplay tests */ 
	char 	*debug_byte_sex;	/* byte sex for X protocol tests */
	int 	debug_visual_check;	/* time delay in X protocol tests */
	int	local;			/* local display server supported */
	int	screen_count;		/* Number of screen server supports */
	char	*visual_classes;	/* The visual class/depth pairs */
	char	*debug_no_pixcheck;	/* Disable pixchecking */
	char	*pixmap_depths;		/* List of pixmap formats */
	char	*server_vendor;		/* returned by XServerVendor */
	int	black_pixel;		/* returned by XBlackPixel */
	int	white_pixel;		/* returned by XWhitePixel */
	int	height_mm;		/* returned by XHeightMMOfScreen */
	int	width_mm;		/* returned by XWidthMMOfScreen */
	int	reset_delay;		/* delay to allow for server reset */
	char	*debug_visual_ids;	/* list of visuals to use */
	int 	extensions;		/* Do we want to use xtest extensions */
};


extern struct config config;

/* Define to allow the use of a more intuitive name for makewin */
#define makedrawable makewin

/*
 * Macros to enclose a region of code that should not abort on an
 * X error.
 */
#define	CATCH_ERROR(DISP)	do {\
		XSync(DISP, False);\
		reseterr();\
		XSetErrorHandler(error_status);\
	} while (0)
#define	RESTORE_ERROR(DISP) do {\
		XSync(DISP, False);\
		XSetErrorHandler(unexp_err);\
	} while (0)
#define	GET_ERROR(DISP)	geterr()	/* ??? */

/* Macro to set the first four elements of an event */
#define defsetevent(EV, D, T) \
	EV.type = T;\
	EV.serial = 0L; \
	EV.send_event = False; \
	EV.display = D;

/* The exit status when a timeout called with settimeout() goes off */
#define	TIMEOUT_EXIT	1
