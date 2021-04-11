/*
 * Captive ctwm handling bits.
 *
 * Captive support makes use of several X properties on various windows.
 *
 * The WM_CTWMSLIST property is set on the root window (of the
 * appropriate Screen) containing a \0-separated list of the names of the
 * captive windows inside that ctwm.  So this would show up in the root
 * window of a captive ctwm as well, if it had more captives inside it.
 *
 * A WM_CTWM_ROOT_<captive_name> property is set on the root window (see
 * previous) for each of the captive ctwm's, holding the Window XID for
 * that captive's internal root window.  The combination of WM_CTWMSLIST
 * and WM_CTWM_ROOT_<name> can be used to find the windows each of the
 * captive ctwms inside us.
 *
 * A WM_CTWM_ROOT is set by the captive ctwm on its created root window,
 * holding the XID of itself.  The same property is also set by the
 * 'outside' ctwm on the frame of that window.  These are used in the
 * f.hypermove process, to find the window ID to move stuff into.  I'm
 * not quite sure why we're setting it on both; perhaps so the border
 * counts as part of the inner window.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xatom.h>
#include <X11/Xmu/WinUtil.h>

#include "captive.h"
#include "events.h"
#include "screen.h"
#include "ctwm_atoms.h"
#include "util.h"
#include "vscreen.h"


static char **GetCaptivesList(int scrnum);
static void SetCaptivesList(int scrnum, char **clist);
static void freeCaptivesList(char **clist);
static Window CaptiveCtwmRootWindow(Window window);
static bool DontRedirect(Window window);

static Atom XA_WM_CTWM_ROOT_our_name;

/* XXX Share with occupation.c? */
static XrmOptionDescRec table [] = {
	{"-xrm",            NULL,           XrmoptionResArg, (XPointer) NULL},
};


/*
 * Reparent a window over to a captive ctwm, if we should.
 */
bool
RedirectToCaptive(Window window)
{
	unsigned long       nitems, bytesafter;
	Atom                actual_type;
	int                 actual_format;
	Bool                status;
	char                *str_type;
	XrmValue            value;
	bool                ret;
	char                *atomname;
	XrmDatabase         db = NULL;

	/* NOREDIRECT property set?  Leave it alone. */
	if(DontRedirect(window)) {
		return false;
	}

	/* Figure out what sort of -xrm stuff it might have */
	{
		char **cliargv = NULL;
		int  cliargc;

		/* Get its command-line */
		if(!XGetCommand(dpy, window, &cliargv, &cliargc)) {
			/* Can't tell, bail */
			return false;
		}

		XrmParseCommand(&db, table, 1, "ctwm", &cliargc, cliargv);
		if(cliargv) {
			XFreeStringList(cliargv);
		}
	}

	/* Bail if we didn't get any info */
	if(db == NULL) {
		return false;
	}

	ret = false;

	/*
	 * Check "-xrm ctwm.redirect" to see if that says what to do.  It
	 * should contain a captive name.  e.g., what ctwm was started with
	 * via --name, or an autogen'd name if no --name was given.
	 * */
	status = XrmGetResource(db, "ctwm.redirect", "Ctwm.Redirect", &str_type,
	                        &value);
	if((status == True) && (value.size != 0)) {
		/* Yep, we're asked for one.  Find it. */
		Window  *prop;
		Atom    XA_WM_CTWM_ROOT_name;
		int     gpret;

		asprintf(&atomname, "WM_CTWM_ROOT_%s", value.addr);
		/*
		 * Set only_if_exists to True: the atom for the requested
		 * captive ctwm won't exist if the captive ctwm itself does not exist.
		 * There is no reason to go and create random atoms just to
		 * check.
		 */
		XA_WM_CTWM_ROOT_name = XInternAtom(dpy, atomname, True);
		free(atomname);

		/*
		 * Got the atom?  Lookup the property it keys for, which holds a
		 * Window identifier.
		 * */
		gpret = !Success;
		prop = NULL;
		if(XA_WM_CTWM_ROOT_name != None) {
			gpret = XGetWindowProperty(dpy, Scr->Root, XA_WM_CTWM_ROOT_name,
			                           0L, 1L, False, AnyPropertyType,
			                           &actual_type, &actual_format,
			                           &nitems, &bytesafter,
			                           (unsigned char **)&prop);
		}

		/*
		 * Got the property?  Make sure it's the right type.  If so, make
		 * sure the window it points at exists.
		 */
		if(gpret == Success
		                && actual_type == XA_WINDOW && actual_format == 32 &&
		                nitems == 1 /*&& bytesafter == 0*/) {
			Window newroot = *prop;
			XWindowAttributes dummy_wa;

			if(XGetWindowAttributes(dpy, newroot, &dummy_wa)) {
				/* Well, it must be where we should redirect to, so do it */
				XReparentWindow(dpy, window, newroot, 0, 0);
				XMapWindow(dpy, window);
				ret = true;
			}
		}
		if(prop != NULL) {
			XFree(prop);
		}

		/* XXX Should we return here if we did the Reparent? */
	}


	/*
	 * Check ctwm.rootWindow; it may contain a (hex) X window identifier,
	 * which we should parent into.
	 * */
	status = XrmGetResource(db, "ctwm.rootWindow", "Ctwm.RootWindow", &str_type,
	                        &value);
	if((status == True) && (value.size != 0)) {
		char rootw [32];
		unsigned long int scanned;

		safe_strncpy(rootw, value.addr, sizeof(rootw));
		if(sscanf(rootw, "%lx", &scanned) == 1) {
			Window newroot = scanned;
			XWindowAttributes dummy_wa;

			if(XGetWindowAttributes(dpy, newroot, &dummy_wa)) {
				XReparentWindow(dpy, window, newroot, 0, 0);
				XMapWindow(dpy, window);
				ret = true;
			}
		}
	}

	/* Cleanup xrm bits */
	XrmDestroyDatabase(db);

	/* Whatever we found */
	return ret;
}


/*
 * Get the list of captive ctwm's we know about on a screen.
 *
 * Use freeCaptivesList() to clean up the return value.
 */
static char **
GetCaptivesList(int scrnum)
{
	unsigned char       *prop, *p;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;
	char                **ret;
	int                 count;
	int                 i, l;
	Window              root;

	root = RootWindow(dpy, scrnum);
	if(XGetWindowProperty(dpy, root, XA_WM_CTWMSLIST, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return NULL;
	}
	if(len == 0) {
		return NULL;
	}

	count = 0;
	p = prop;
	l = 0;
	while(l < len) {
		l += strlen((char *)p) + 1;
		p += strlen((char *)p) + 1;
		count++;
	}
	ret = calloc(count + 1, sizeof(char *));

	p = prop;
	l = 0;
	i = 0;
	while(l < len) {
		ret [i++] = strdup((char *) p);
		l += strlen((char *)p) + 1;
		p += strlen((char *)p) + 1;
	}
	ret [i] = NULL;
	XFree(prop);

	return ret;
}


/*
 * Free GetCaptivesList() return.
 */
static void
freeCaptivesList(char **clist)
{
	if(clist == NULL) {
		return;
	}

	for(char **tmp = clist ; *tmp != NULL ; tmp++) {
		free(*tmp);
	}

	free(clist);
}


/*
 * Set the WM_CTWMSLIST property with a set of captive ctwm's, so it can
 * be retrieved from there later (say, by a GetCaptivesList() call).
 */
static void
SetCaptivesList(int scrnum, char **clist)
{
	unsigned long       len;
	char                **cl;
	char                *s, *slist;
	Window              root = RootWindow(dpy, scrnum);

	cl  = clist;
	len = 0;
	while(*cl) {
		len += strlen(*cl++) + 1;
	}
	if(len == 0) {
		XDeleteProperty(dpy, root, XA_WM_CTWMSLIST);
		return;
	}
	slist = calloc(len, sizeof(char));
	cl = clist;
	s  = slist;
	while(*cl) {
		strcpy(s, *cl);
		s += strlen(*cl);
		*s++ = '\0';
		cl++;
	}
	XChangeProperty(dpy, root, XA_WM_CTWMSLIST, XA_STRING, 8,
	                PropModeReplace, (unsigned char *) slist, len);
	free(slist);
}


/*
 * Add ourselves to the list of captive ctwms.  Called during startup
 * when --window is given.  Returns the captive name, because we may not
 * have been given one explicitly (in cptname), and so it may have been
 * autogen'd.
 */
char *
AddToCaptiveList(const char *cptname)
{
	int         i, count;
	char        **clist, **cl, **newclist;
	int         busy [32];
	char        *atomname;
	int         scrnum = Scr->screen;
	Window      croot  = Scr->Root;
	Window      root;
	char        *rcname;

	for(i = 0; i < 32; i++) {
		busy [i] = 0;
	}

	/*
	 * Go through our current captives to see what's taken
	 */
	clist = GetCaptivesList(scrnum);
	cl = clist;
	count = 0;
	while(cl && *cl) {
		count++;

		/*
		 * If we're not given a cptname, we use this loop to mark up
		 * which auto-gen'd names have been used.
		 */
		if(!cptname) {
			if(!strncmp(*cl, "ctwm-", 5)) {
				int r, n;
				r = sscanf(*cl, "ctwm-%d", &n);
				cl++;
				if(r != 1) {
					continue;
				}
				if((n < 0) || (n > 31)) {
					continue;
				}
				busy [n] = 1;
			}
			else {
				cl++;
			}
			continue;
		}

		/*
		 * If we do have a cptname, and a captive already has the
		 * requested name, bomb
		 */
		if(!strcmp(*cl, cptname)) {
			fprintf(stderr, "A captive ctwm with name %s is already running\n",
			        cptname);
			exit(1);
		}
		cl++;
	}


	/*
	 * If we weren't given a name, find an available autogen one.  If we
	 * were, just dup it for our return value.
	 */
	if(!cptname) {
		for(i = 0; i < 32; i++) {
			if(!busy [i]) {
				break;
			}
		}
		if(i == 32) {  /* no one can tell we didn't try hard */
			fprintf(stderr, "Cannot find a suitable name for captive ctwm\n");
			exit(1);
		}
		asprintf(&rcname, "ctwm-%d", i);
	}
	else {
		rcname = strdup(cptname);
		if(rcname == NULL) {
			fprintf(stderr, "strdup() for rcname failed!\n");
			abort();
		}
	}


	/* Put together new list of captives */
	newclist = calloc(count + 2, sizeof(char *));
	for(i = 0; i < count; i++) {
		newclist[i] = strdup(clist[i]);
	}
	newclist[count] = strdup(rcname);
	newclist[count + 1] = NULL;
	SetCaptivesList(scrnum, newclist);
	freeCaptivesList(clist);
	freeCaptivesList(newclist);

	/* Stash property/atom of our captivename */
	root = RootWindow(dpy, scrnum);
	asprintf(&atomname, "WM_CTWM_ROOT_%s", rcname);
	XA_WM_CTWM_ROOT_our_name = XInternAtom(dpy, atomname, False);
	free(atomname);
	XChangeProperty(dpy, root, XA_WM_CTWM_ROOT_our_name, XA_WINDOW, 32,
	                PropModeReplace, (unsigned char *) &croot, 1);

	/*
	 * Tell our caller the name we wound up with, in case they didn't
	 * give us one we could use.
	 */
	return rcname;
}


/*
 * Take something (in practice, always ourselves) out of the list of
 * running captives.  Called during shutdown.
 */
void
RemoveFromCaptiveList(const char *cptname)
{
	char **clist;
	int scrnum = Scr->screen;
	Window root = RootWindow(dpy, scrnum);

	/* If we're not apparently captive, there's nothing to do */
	if(!cptname || XA_WM_CTWM_ROOT_our_name == None) {
		return;
	}

	/* Take us out of the captives list in WM_CTWMSLIST */
	clist = GetCaptivesList(scrnum);
	if(clist && *clist) {
		char **cl = clist;
		char **newclist;
		int  count;
		bool found;

		/* How many are there? */
		count = 0;
		found = false;
		while(*cl) {
			if(strcmp(*cl, cptname) == 0) {
				found = true;
			}
			count++;
			cl++;
		}

		/* If we weren't there, there's nothing to do */
		if(!found) {
			freeCaptivesList(clist);
			return;
		}

		/*
		 * Make a new list without cptname in it.  A list with (count)
		 * items needs (count+1) for the trailing NULL, but we know we're
		 * in it and removing ourself, so we only need ((count-1)+1).
		 *
		 * Note that we're _not_ strdup()'ing into newclist, just
		 * sticking a pointer to the existing string inside clist.  Then
		 * we only have to free() newclist itself, because there's
		 * nothing new inside it.  We explicitly do _NOT_ want to
		 * freeCaptivesList() it, since that would free the internals,
		 * and then when we fCL(clist) it would try to double-free them.
		 */
		newclist = calloc(count, sizeof(char *));
		cl = clist;
		count = 0;
		while(*cl) {
			if(!strcmp(*cl, cptname)) {
				cl++;
				continue;
			}
			newclist [count++] = *cl;
			cl++;
		}
		newclist [count] = NULL;
		SetCaptivesList(scrnum, newclist);
		free(newclist);
	}
	freeCaptivesList(clist);

	/* And delete our CTWM_ROOT_x property */
	XDeleteProperty(dpy, root, XA_WM_CTWM_ROOT_our_name);
}


/*
 * Call from AddWindow() on the 'outside'; if this new window is a
 * captive ctwm running inside us, copy its WM_CTWM_ROOT property to the
 * frame window we're creating around it.  It's a little unclear why
 * we're doing this; x-ref comment at top of file.
 */
void
SetPropsIfCaptiveCtwm(TwmWindow *win)
{
	Window      window = win->w;
	Window      frame  = win->frame;

	if(!CaptiveCtwmRootWindow(window)) {
		return;
	}

	XChangeProperty(dpy, frame, XA_WM_CTWM_ROOT, XA_WINDOW, 32,
	                PropModeReplace, (unsigned char *) &window, 1);
}


/*
 * Get the WM_CTWM_ROOT property of a window; that tells us whether it
 * thinks it's a captive ctwm, and if so, what it thinks its root window
 * is.
 */
static Window
CaptiveCtwmRootWindow(Window window)
{
	Window             *prop;
	Window              w;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;

	if(XGetWindowProperty(dpy, window, XA_WM_CTWM_ROOT, 0L, 1L,
	                      False, XA_WINDOW, &actual_type, &actual_format, &len,
	                      &bytesafter, (unsigned char **)&prop) != Success) {
		return ((Window)0);
	}
	if(len == 0) {
		return ((Window)0);
	}
	w = *prop;
	XFree(prop);
	return w;
}


/*
 * Get info about the captive CTWM instance under the cursor.  Called
 * during the f.hypermove process.
 */
CaptiveCTWM
GetCaptiveCTWMUnderPointer(void)
{
	Window      root;
	Window      child, croot;
	CaptiveCTWM cctwm;
	char        *rname;

	root = RootWindow(dpy, Scr->screen);
	while(1) {
		XQueryPointer(dpy, root, &JunkRoot, &child,
		              &JunkX, &JunkY, &JunkX, &JunkY, &JunkMask);
		if(child && (croot = CaptiveCtwmRootWindow(child))) {
			root = croot;
			continue;
		}
		cctwm.root = root;

		/*
		 * We indirect through the extra var here for probably
		 * unnecessary reasons; X resources (like that from XFetchName)
		 * are specified to be freed via XFree(), not via free().  And we
		 * don't want our callers to have to know that (or worse, know to
		 * do it SOMEtimes, since we also might create it ourselves with
		 * strdup()).  So eat the extra allocation/copy and insulate
		 * callers.
		 */
		XFetchName(dpy, root, &rname);
		if(rname) {
			cctwm.name = strdup(rname);
			XFree(rname);
		}
		else {
			cctwm.name = strdup("Root");
		}

		return (cctwm);
	}
}


/*
 * We set a NOREDIRECT property on windows in certain situations as a
 * result of a f.hypermove.  That gets checked during
 * RedirectToCaptive(), causing it to to not mess with the window.
 *
 * XXX I'm not sure this actually makes any sense; RTC() only gets called
 * at the beginning of AddWindow(), only if ctwm isn't running captive.
 * So the upshot is that this causes AddWindow() to do nothing and return
 * NULL, in the case that a window was hypermoved from a captive ctwm
 * into a non-captive ctwm.
 *
 * That's OK I think, because all the AddWindow() stuff would have
 * already been done for it, so there's nothing to do?  But this suggests
 * that there's leakage happening; we keep a TwmWindow struct around in
 * the "old" ctwm when it's moved into a new one, and since AddWindow()
 * only does the condition if we're a non-captive ctwm, it means the
 * _captive_ ctwm recreates a new one every time it's hypermoved in?
 */
void
SetNoRedirect(Window window)
{
	XChangeProperty(dpy, window, XA_WM_NOREDIRECT, XA_STRING, 8,
	                PropModeReplace, (unsigned char *) "Yes", 4);
}

static bool
DontRedirect(Window window)
{
	unsigned char       *prop;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;

	if(XGetWindowProperty(dpy, window, XA_WM_NOREDIRECT, 0L, 1L,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return false;
	}
	if(len == 0) {
		return false;
	}
	XFree(prop);
	return true;
}


/*
 * Handling of a ConfigureNotify for a captive root window.
 */
void
ConfigureCaptiveRootWindow(XEvent *ev)
{
	Window       root, child;
	int          x, y;
	unsigned int w, h, bw, d, oldw, oldh;

	/* Guard */
	if(!CLarg.is_captive) {
		fprintf(stderr, "BUG: %s(): Shouldn't get called unless captive.\n",
		        __func__);
		return;
	}

	XGetGeometry(dpy, Scr->CaptiveRoot, &root, &x, &y, &w, &h, &bw, &d);
	XTranslateCoordinates(dpy, Scr->CaptiveRoot, root, 0, 0, &Scr->crootx,
	                      &Scr->crooty, &child);

	oldw = Scr->crootw;
	oldh = Scr->crooth;
	Scr->crootw = ev->xconfigure.width;
	Scr->crooth = ev->xconfigure.height;
#if 0
	fprintf(stderr, "%s(): cx = %d, cy = %d, cw = %d, ch = %d\n",
	        __func__, Scr->crootx, Scr->crooty, Scr->crootw, Scr->crooth);
#endif
	if(Scr->currentvs) {
		Scr->rootx = Scr->crootx + Scr->currentvs->x;
		Scr->rooty = Scr->crooty + Scr->currentvs->y;
	}
	Scr->rootw = Scr->crootw;
	Scr->rooth = Scr->crooth;

	/*
	 * XXX This is a little weird, and in my experience _always_ triggers
	 * when a captive window starts up.  So what's the point?
	 */
	if((Scr->crootw != oldw) || (Scr->crooth != oldh)) {
		fprintf(stderr, "%s: You cannot change root window geometry "
		        "with virtual screens active,\n"
		        "from now on, the ctwm behaviour is unpredictable.\n",
		        ProgramName);
	}
}


/*
 * Adopt a window into the calling captive ctwm.  Backend for
 * f.adoptwindow function.
 */
void AdoptWindow(void)
{
	unsigned long       data [2];
	Window              localroot, w;
	unsigned char       *prop;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;
	XEvent              event;
	Window              root, parent, child, *children;
	unsigned int        nchildren, key_buttons;
	int                 root_x, root_y, win_x, win_y;
	int                 ret;
	bool                savedRestartPreviousState;

	localroot = w = RootWindow(dpy, Scr->screen);
	XGrabPointer(dpy, localroot, False,
	             ButtonPressMask | ButtonReleaseMask,
	             GrabModeAsync, GrabModeAsync,
	             None, Scr->SelectCursor, CurrentTime);

	XMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask, &event);
	child = event.xbutton.subwindow;
	while(1) {
		if(child == (Window) 0) {
			break;
		}

		w = XmuClientWindow(dpy, child);
		ret = XGetWindowProperty(dpy, w, XA_WM_WORKSPACESLIST, 0L, 512,
		                         False, XA_STRING, &actual_type, &actual_format, &len,
		                         &bytesafter, &prop);
		XFree(prop);  /* Don't ever do anything with it */
		if(ret != Success) {
			break;
		}
		if(len == 0) { /* it is not a local root window */
			break;        /* it is not a local root window */
		}
		localroot = w;
		XQueryPointer(dpy, localroot, &root, &child, &root_x, &root_y,
		              &win_x, &win_y, &key_buttons);
	}
	XMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask, &event);
	XUngrabPointer(dpy, CurrentTime);

	if(localroot == Scr->Root) {
		return;
	}
	if(w == localroot) {   /* try to not adopt an ancestor */
		XQueryTree(dpy, Scr->Root, &root, &parent, &children, &nchildren);
		while(parent != (Window) 0) {
			XFree(children);
			if(w == parent) {
				return;
			}
			XQueryTree(dpy, parent, &root, &parent, &children, &nchildren);
		}
		XFree(children);
		if(w == root) {
			return;
		}
	}
	if(localroot == RootWindow(dpy, Scr->screen)) {
		XWithdrawWindow(dpy, w, Scr->screen);
	}
	else {
		XUnmapWindow(dpy, w);
	}
	XReparentWindow(dpy, w, Scr->Root, 0, 0);

	data [0] = (unsigned long) NormalState;
	data [1] = (unsigned long) None;

	XChangeProperty(dpy, w, XA_WM_STATE, XA_WM_STATE, 32,
	                PropModeReplace, (unsigned char *) data, 2);
	XFlush(dpy);
	/*
	 * We don't want to "restore" the occupation that the window had
	 * in its former environment. For one, the names of the workspaces
	 * may be different. And if not, the window will initially be
	 * shown in the current workspace, which may be at odds with that
	 * occupation (and confusion ensues).
	 *
	 * Hypermove has the same problem, but that is a "push" operation
	 * (initiated by the originating window manager) so we don't know
	 * when it happens...
	 */
	savedRestartPreviousState = RestartPreviousState;
	RestartPreviousState = false;
	SimulateMapRequest(w);
	RestartPreviousState = savedRestartPreviousState;
	return;
}
