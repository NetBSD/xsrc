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
 * $XConsortium: xtestlib.h,v 1.24 94/04/17 21:01:09 rws Exp $
 */


/*
 * Structure to define an area by position and size.
 */
struct	area {
	int 	x;
	int 	y;
	unsigned int 	width;
	unsigned int 	height;
};

/*
 * Flags for the checkarea() function.
 */
#define	CHECK_IN	0x1		/* Check inside the area */
#define CHECK_OUT	0x2		/* Check outside the area */
#define CHECK_ALL	(CHECK_IN|CHECK_OUT)	/* Check both in and out */
#define	CHECK_DIFFER	0x4	/* Check that areas differ */

/*
 * Flags for the resetvinf() function.
 */
#define	VI_WIN		0x1		/* Return visuals for windows */
#define	VI_PIX		0x2		/* Return depths for pixmaps */
#define	VI_WIN_PIX	(VI_WIN|VI_PIX)	/* Return visuals and depths */
#define VI_ALT_WIN	0x4		/* Return visuals for windows
					 	on alternate screen */
#define	VI_ALT_PIX	0x8		/* Return depts for pixmaps
						on alternate screen */
#define	VI_ALT_WIN_PIX	(VI_ALT_WIN|VI_ALT_PIX)
					/* Return alternate visuals & depths */

/*
 * Defines for the notmember() function that is used to test BadValue
 */
#define NM_GREATER	0
#define	NM_LESS		1
#define	NM_NEGATIVE	2
#define	NM_LARGE	3
#define	NM_LEN		4

/*
 * Structure to hold the connections between names, XFontStruct's and
 * the associated string.
 */
struct	fontinfo	{
	char	*name;		/* Font name */
	XFontStruct	*fontstruct;	/* pointer to known good XFontStruct */
	char	**string;	/* Copyright string */
	short	flag;	/* for use by lstfnt* */
};

/*
 * The following defines and typedef's are related to block().
 */

#define	BLOCK_FILE	"block_file"
/*
 * predicate procedure argument types
 */
typedef	Bool (*Predicate) (
#if TEST_ANSI
	Display *display,
	XEvent	*error_event,
	char	*arg
#endif
);

/*
 * The type of the procedure argument to block().
 *
 * It should be noted that some of these function definitions are
 * listed as returning int's where the return value is not actually
 * specified by Xlib (so, they therefore default to int).
 */
typedef union {
	int	(*a0)(
#if TEST_ANSI
		/*
		 * The arguments have been commented out at present, because
		 * some compilers have problems with this.
		 */
		/* Display *d, int	a */
#endif
	);
	int	(*a1)(
#if TEST_ANSI
		/* Display *d, XEvent *e, Predicate p, char *cp */
#endif
	);
	int	(*a2)(
#if TEST_ANSI
		/* Display *d, long l, XEvent *ep */
#endif
	);
	int	(*a3)(
#if TEST_ANSI
		/* Display *d, XEvent *e */
#endif
	);
	int	(*a4)(
#if TEST_ANSI
		/* Display *d, Window w, long l, XEvent *ep */
#endif
	);
	int	(*a5)(
#if TEST_ANSI
		/* Display *d */
#endif
	);
} Block_Proc;

/*
 * Convenience constants for accessing various types of function pointers
 */
#define	XEventsQueued_Like_Proc	blocker.a0
#define	XIfEvent_Like_Proc	blocker.a1
#define	XMaskEvent_Like_Proc	blocker.a2
#define	XNextEvent_Like_Proc	blocker.a3
#define	XPeekEvent_Like_Proc	blocker.a3
#define	XPeekIfEvent_Like_Proc	blocker.a1
#define	XWindowEvent_Like_Proc	blocker.a4
#define	XPending_Like_Proc	blocker.a5

/*
 * The type of the info argument to block().
 */
typedef struct _Block_Info {
	int	p_type;
	Block_Proc	blocker;
	int	int_return;
	XEvent	event_return;
	union {
		struct {	/* XEventsQueued */
			int		mode;
		} a0;
		struct {	/* XIfEvent, XPeekIfEvent */
			Predicate	predicate;
			char*		arg;
		} a1;
		struct {	/* XMaskEvent */
			long		event_mask;
		} a2;
		struct {	/* XWindowEvent */
			Window		w;
			long		event_mask;
		} a3;
	} u;
} *Block_InfoP, Block_Info;

/*
 * Convenience constants for getting at the various argument types.
 */
#define	XEventsQueued_Args	u.a0
#define	XIfEvent_Args		u.a1
#define	XMaskEvent_Args		u.a2
/*
 * These have no (and need no) corresponding entry in the argument union.
#define	XNextEvent_Args
#define	XPeekEvent_Args
#define	XPending_Args
*/
#define	XPeekIfEvent_Args	u.a1
#define	XWindowEvent_Args	u.a3

/*
 * Convenience constants for checking the various argument types.
 */
#define	Ignore_Event_Return	(1<<15)
#define	XEventsQueued_Like	((1<<0)|Ignore_Event_Return)
#define	XIfEvent_Like		(1<<1)
#define	XMaskEvent_Like		(1<<2)
#define	XNextEvent_Like		(1<<3)
#define	XPeekEvent_Like		(1<<4)
#define	XPeekIfEvent_Like	(1<<5)
#define	XWindowEvent_Like	(1<<6)
#define	XPending_Like		((1<<7)|Ignore_Event_Return)

/*
 * Macros to set some of the info fields
 */
#define	XEventsQueued_Type(i, m)	\
	i.XEventsQueued_Like_Proc = XEventsQueued;\
	i.p_type = XEventsQueued_Like;\
	i.XEventsQueued_Args.mode = m
#define	XIfEvent_Type(i, p, a)	\
	i.XIfEvent_Like_Proc = XIfEvent;\
	i.p_type = XIfEvent_Like;\
	i.XIfEvent_Args.predicate = p;\
	i.XIfEvent_Args.arg = a
#define	XMaskEvent_Type(i, m)	\
	i.XMaskEvent_Like_Proc = XMaskEvent;\
	i.p_type = XMaskEvent_Like;\
	i.XMaskEvent_Args.event_mask = m
#define	XNextEvent_Type(i)	\
	i.XNextEvent_Like_Proc = XNextEvent;\
	i.p_type = XNextEvent_Like
#define	XPeekEvent_Type(i)	\
	i.XPeekEvent_Like_Proc = XPeekEvent;\
	i.p_type = XPeekEvent_Like
#define	XPeekIfEvent_Type(i, p, a)	\
	i.XPeekIfEvent_Like_Proc = XPeekIfEvent;\
	i.p_type = XPeekIfEvent_Like;\
	i.XPeekIfEvent_Args.predicate = p;\
	i.XPeekIfEvent_Args.arg = a
#define	XWindowEvent_Type(i, w, m)	\
	i.XWindowEvent_Like_Proc = XWindowEvent;\
	i.p_type = XWindowEvent_Like;\
	i.XWindowEvent_Args.w = w;\
	i.XWindowEvent_Args.event_mask = m
#define	XPending_Type(i)	\
	i.XPending_Like_Proc = XPending;\
	i.p_type = XPending_Like

/*
 * The following defines and typedef's are related to winh.
 */
typedef	struct {
	int	high;
	int	low;
	int	count;
} Winhs;

extern	Winhs	winh_event_stats[];

typedef	struct	_Winhc {
	Display	*display;	/* client identifier */
	struct _Winhc	*next;	/* next client in client list */
	struct _Winh	*node;	/* pointer to member in hierarchy */
	long	event_mask;	/* mask of selected events */
	long	flags;		/* used during hierarchy checking & traversal */
} Winhc;

typedef	struct	_Winhe {
	XEvent	*event;		/* an event */
	struct _Winhe	*next;	/* next associated event */
	long	sequence;	/* sequence number */
	long	flags;		/* used during hierarchy checking & traversal */
} Winhe;

typedef	struct	_Winhg {
	struct area area;
	int	border_width;	/* */
} Winhg;

typedef	struct	_Winh {
	Window	window;		/* window ID */
	struct _Winh	*parent;	/* ptr to parent in hierarchy, NULL if top */
	struct _Winh	*nextsibling;	/* linked list of siblings */
	struct _Winh	*prevsibling;	/* linked list of siblings */
	struct _Winh	*firstchild;	/* linked list of children */
	int	numchildren;	/* number of children */
	unsigned long	valuemask;	/* mask corresponding to attributes */
	XSetWindowAttributes	attributes;	/* */
	long	winhmask;	/* Winh-specific flags */
	Winhc	*clients;	/* linked list of interested clients */
	Winhe	*expected;	/* linked list of expected events */
	Winhe	*delivered;	/* linked list of delivered events */
	int	depth;		/* depth in window hierarchy */
	Winhg	winhg;		/* geometry hook */
	int	screen;		/* only used for children of guardian */
} Winh;

extern	Winh	*guardian;
extern	Winhe	*winh_qexp;
extern	Winhe	*winh_qdel;

/*
 * winhmask values
 */
#define	WINH_NOMASK		(0L<<0)
#define	WINH_CREATED		(1L<<0)	/* window of node created */
#define	WINH_DEL_PROPOGATE	(1L<<1)	/* event propagates */
#define	WINH_DEL_SEND_EVENT	(1L<<2)	/* behave as would XSendEvent */
#define	WINH_GUARDIAN		(1L<<3)	/* a guardian node */
#define	WINH_INHERIT		(1L<<4)	/* use winhmask values from parent */
#define	WINH_MAP		(1L<<5)	/* map upon creation */
#define	WINH_WEED_IDENTITY	(1L<<6)	/* expected should equal delivered */
#define	WINH_WEED_MINIMUM	(1L<<7)
#define	WINH_WEED_TYPE		(1L<<8)
#define	WINH_IGNORE_GEOMETRY	(1L<<9)	/* winhg not initialized */
#define	WINH_BOTH_SCREENS	(1L<<10)/* build hierarchy on both screens */

#define	WINH_BAD	((Window) -1)

/*
 * Pointer location management structure (pointer.c)
 */
typedef	struct {
	Window	oroot;	/* where the pointer was */
	int	ox, oy;
	Window	nroot;	/* where the pointer moved to */
	int	nx, ny;
} PointerPlace;

/*
 * Defines for the resource registering service.
 */
#define	REG_IMAGE	0
#define	REG_WINDOW	1
#define	REG_PIXMAP	2
#define	REG_GC		3
#define	REG_COLORMAP	4
#define	REG_CURSOR	5
#define	REG_OPEN	6
#define	REG_WINH	7
#define	REG_POINTER	8
#define	REG_MALLOC	9
#define	REG_XMALLOC	10

#define	REG_REGION	11
#define	REG_MAX		12	

/* union of types that can be registered */
union	regtypes {
	XImage	*image;
	Window	window;
	Pixmap	pixmap;
	GC		gc;
	Colormap	colormap;
	Cursor	cursor;
	Display	*display;
	Winh	*winh;
	PointerPlace	*pointer;
	char	*malloc;
	Region region;
};

/*
 * Some event convenience macros.
 */

/* must be called after window, x, and y members are set! */
#define	ROOTCOORDSET(d, e)	\
	rootcoordset((d), (e)->window, DRW((d)), (e)->x, (e)->y, &((e)->x_root), &((e)->y_root))

/* used when one wishes to check serial */
#define CHECKEVENT(g, e)        \
	if (checkevent((g), (e)) || serialtest((g), (e)) == False) {\
		report("Delivered event did not match expected event");\
		FAIL;\
	}\
	else\
		CHECK

/*
 * Structure for use with buildtree.
 */
struct	buildtree	{
	char	*name;	/* Name */
	char	*pname;	/* Parent's name */
	Window	wid;	/* Window id */
	struct buildtree *parent;	/* pointer to parent */
	unsigned long	uflags;	/* Flags for user of routines */
	unsigned long	opts;	/* Internal flags renamed from 'flags' */
	int 	num;	/* number in list (only valid in list[0]) */
	int 	x, y;	/* Position */
	unsigned int 	width, height;	/* size */
	unsigned int 	borderwidth;	/* border width */
};

/*
 * Flags for use in buildtree.
 */
#define	BT_UNMAP	1	/* Window is unmapped */

/*
 * A structure for linking values and names.  Used in the functions defined
 * in lookupname.c.
 */
struct	valname	{
	int 	val;
	char	*name;
};

/*
 * Include the automaticaly generated prototypes.
 */
#include	"xtlibproto.h"
