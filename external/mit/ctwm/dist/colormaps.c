/*
 * Colormap handling
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include "colormaps.h"
#include "screen.h"


/*
 * From events.c; imported manually since I'm not listing it in events.h
 * because nowhere but here needs it.
 */
extern bool ColortableThrashing;

static Bool UninstallRootColormapQScanner(Display *display, XEvent *ev,
                char *args);


/***********************************************************************
 *
 *  Procedure:
 *      InstallWindowColormaps - install the colormaps for one twm window
 *
 *  Inputs:
 *      type    - type of event that caused the installation
 *      tmp     - for a subset of event types, the address of the
 *                window structure, whose colormaps are to be installed.
 *
 ***********************************************************************
 *
 * Previously in events.c
 */
bool
InstallWindowColormaps(int type, TwmWindow *tmp)
{
	if(tmp) {
		return InstallColormaps(type, &tmp->cmaps);
	}
	else {
		return InstallColormaps(type, NULL);
	}
}


bool
InstallColormaps(int type, Colormaps *cmaps)
{
	int i, j, n, number_cwins, state;
	ColormapWindow **cwins, *cwin, **maxcwin = NULL;
	TwmColormap *cmap;
	char *row, *scoreboard;

	switch(type) {
		case EnterNotify:
		case LeaveNotify:
		case DestroyNotify:
		default:
			/* Save the colormap to be loaded for when force loading of
			 * root colormap(s) ends.
			 */
			Scr->cmapInfo.pushed_cmaps = cmaps;
			/* Don't load any new colormap if root colormap(s) has been
			 * force loaded.
			 */
			if(Scr->cmapInfo.root_pushes) {
				return false;
			}
			/* Don't reload the current window colormap list.
			if (Scr->cmapInfo.cmaps == cmaps)
			    return false;
			 */
			if(Scr->cmapInfo.cmaps) {
				for(i = Scr->cmapInfo.cmaps->number_cwins,
				                cwins = Scr->cmapInfo.cmaps->cwins; i-- > 0; cwins++) {
					(*cwins)->colormap->state &= ~CM_INSTALLABLE;
				}
			}
			Scr->cmapInfo.cmaps = cmaps;
			break;

		case PropertyNotify:
		case VisibilityNotify:
		case ColormapNotify:
			break;
	}

	number_cwins = Scr->cmapInfo.cmaps->number_cwins;
	cwins = Scr->cmapInfo.cmaps->cwins;
	scoreboard = Scr->cmapInfo.cmaps->scoreboard;

	ColortableThrashing = false; /* in case installation aborted */

	state = CM_INSTALLED;

	for(i = n = 0; i < number_cwins; i++) {
		cwins[i]->colormap->state &= ~CM_INSTALL;
	}
	for(i = n = 0; i < number_cwins && n < Scr->cmapInfo.maxCmaps; i++) {
		cwin = cwins[i];
		cmap = cwin->colormap;
		if(cmap->state & CM_INSTALL) {
			continue;
		}
		cmap->state |= CM_INSTALLABLE;
		cmap->w = cwin->w;
		if(cwin->visibility != VisibilityFullyObscured) {
			row = scoreboard + (i * (i - 1) / 2);
			for(j = 0; j < i; j++)
				if(row[j] && (cwins[j]->colormap->state & CM_INSTALL)) {
					break;
				}
			if(j != i) {
				continue;
			}
			n++;
			maxcwin = &cwins[i];
			state &= (cmap->state & CM_INSTALLED);
			cmap->state |= CM_INSTALL;
		}
	}
	Scr->cmapInfo.first_req = NextRequest(dpy);

	for(; n > 0 && maxcwin >= &cwins[0]; maxcwin--) {
		cmap = (*maxcwin)->colormap;
		if(cmap->state & CM_INSTALL) {
			cmap->state &= ~CM_INSTALL;
			if(!(state & CM_INSTALLED)) {
				cmap->install_req = NextRequest(dpy);
				/* printf ("XInstallColormap : %x, %x\n", cmap, cmap->c); */
				XInstallColormap(dpy, cmap->c);
			}
			cmap->state |= CM_INSTALLED;
			n--;
		}
	}
	return true;
}



/***********************************************************************
 *
 *  Procedures:
 *      <Uni/I>nstallRootColormap - Force (un)loads root colormap(s)
 *
 *         These matching routines provide a mechanism to insure that
 *         the root colormap(s) is installed during operations like
 *         rubber banding or menu display that require colors from
 *         that colormap.  Calls may be nested arbitrarily deeply,
 *         as long as there is one UninstallRootColormap call per
 *         InstallRootColormap call.
 *
 *         The final UninstallRootColormap will cause the colormap list
 *         which would otherwise have be loaded to be loaded, unless
 *         Enter or Leave Notify events are queued, indicating some
 *         other colormap list would potentially be loaded anyway.
 ***********************************************************************
 *
 * Previously in events.c
 */
void
InstallRootColormap(void)
{
	Colormaps *tmp;
	if(Scr->cmapInfo.root_pushes == 0) {
		/*
		 * The saving and restoring of cmapInfo.pushed_window here
		 * is a slimy way to remember the actual pushed list and
		 * not that of the root window.
		 */
		tmp = Scr->cmapInfo.pushed_cmaps;
		InstallColormaps(0, &Scr->RootColormaps);
		Scr->cmapInfo.pushed_cmaps = tmp;
	}
	Scr->cmapInfo.root_pushes++;
}


/* ARGSUSED*/
static Bool
UninstallRootColormapQScanner(Display *display, XEvent *ev,
                              char *args)
{
	if(!*args) {
		if(ev->type == EnterNotify) {
			if(ev->xcrossing.mode != NotifyGrab) {
				*args = 1;
			}
		}
		else if(ev->type == LeaveNotify) {
			if(ev->xcrossing.mode == NotifyNormal) {
				*args = 1;
			}
		}
	}

	return (False);
}


void
UninstallRootColormap(void)
{
	char args;
	XEvent dummy;

	if(Scr->cmapInfo.root_pushes) {
		Scr->cmapInfo.root_pushes--;
	}

	if(!Scr->cmapInfo.root_pushes) {
		/*
		 * If we have subsequent Enter or Leave Notify events,
		 * we can skip the reload of pushed colormaps.
		 */
		XSync(dpy, 0);
		args = 0;
		XCheckIfEvent(dpy, &dummy, UninstallRootColormapQScanner, &args);

		if(!args) {
			InstallColormaps(0, Scr->cmapInfo.pushed_cmaps);
		}
	}
}


/*
 * Create a TwmColormap struct and tie it to an [X] Colormap.  Places
 * that need to mess with colormaps and look up the metainfo we hang off
 * them need to look this up and find it via the X Context.
 *
 * Previously in add_window.c
 */
TwmColormap *
CreateTwmColormap(Colormap c)
{
	TwmColormap *cmap;
	cmap = malloc(sizeof(TwmColormap));
	if(!cmap || XSaveContext(dpy, c, ColormapContext, (XPointer) cmap)) {
		if(cmap) {
			free(cmap);
		}
		return (NULL);
	}
	cmap->c = c;
	cmap->state = 0;
	cmap->install_req = 0;
	cmap->w = None;
	cmap->refcnt = 1;
	return (cmap);
}


/*
 * Put together a ColormapWindow struct.  This is a thing we hang off a
 * TwmWindow for some colormap tracking stuff.
 *
 * Previously in add_window.c
 */
ColormapWindow *
CreateColormapWindow(Window w, bool creating_parent, bool property_window)
{
	ColormapWindow *cwin;
	TwmColormap *cmap;
	XWindowAttributes attributes;

	cwin = malloc(sizeof(ColormapWindow));
	if(cwin) {
		if(!XGetWindowAttributes(dpy, w, &attributes) ||
		                XSaveContext(dpy, w, ColormapContext, (XPointer) cwin)) {
			free(cwin);
			return (NULL);
		}

		if(XFindContext(dpy, attributes.colormap,  ColormapContext,
		                (XPointer *)&cwin->colormap) == XCNOENT) {
			cwin->colormap = cmap = CreateTwmColormap(attributes.colormap);
			if(!cmap) {
				XDeleteContext(dpy, w, ColormapContext);
				free(cwin);
				return (NULL);
			}
		}
		else {
			cwin->colormap->refcnt++;
		}

		cwin->w = w;
		/*
		 * Assume that windows in colormap list are
		 * obscured if we are creating the parent window.
		 * Otherwise, we assume they are unobscured.
		 */
		cwin->visibility = creating_parent ?
		                   VisibilityPartiallyObscured : VisibilityUnobscured;
		cwin->refcnt = 1;

		/*
		 * If this is a ColormapWindow property window and we
		 * are not monitoring ColormapNotify or VisibilityNotify
		 * events, we need to.
		 */
		if(property_window &&
		                (attributes.your_event_mask &
		                 (ColormapChangeMask | VisibilityChangeMask)) !=
		                (ColormapChangeMask | VisibilityChangeMask)) {
			XSelectInput(dpy, w, attributes.your_event_mask |
			             (ColormapChangeMask | VisibilityChangeMask));
		}
	}

	return (cwin);
}


/*
 * Do something with looking up stuff from WM_COLORMAPS_WINDOWS (relating
 * to windows with their own colormap) and finding or putting this window
 * into it.
 *
 * XXX Someone should figure it out better than that...
 *
 * Previously in add_window.c
 */
void
FetchWmColormapWindows(TwmWindow *tmp)
{
	int i, j;
	Window *cmap_windows = NULL;
	bool can_free_cmap_windows = false;
	int number_cmap_windows = 0;
	ColormapWindow **cwins = NULL;
	bool previnst;

	number_cmap_windows = 0;

	previnst = (Scr->cmapInfo.cmaps == &tmp->cmaps && tmp->cmaps.number_cwins);
	if(previnst) {
		cwins = tmp->cmaps.cwins;
		for(i = 0; i < tmp->cmaps.number_cwins; i++) {
			cwins[i]->colormap->state = 0;
		}
	}

	if(XGetWMColormapWindows(dpy, tmp->w, &cmap_windows,
	                         &number_cmap_windows) &&
	                number_cmap_windows > 0) {

		/*
		 * check if the top level is in the list, add to front if not
		 */
		for(i = 0; i < number_cmap_windows; i++) {
			if(cmap_windows[i] == tmp->w) {
				break;
			}
		}
		if(i == number_cmap_windows) {   /* not in list */
			Window *new_cmap_windows =
			        calloc((number_cmap_windows + 1), sizeof(Window));

			if(!new_cmap_windows) {
				fprintf(stderr,
				        "%s:  unable to allocate %d element colormap window array\n",
				        ProgramName, number_cmap_windows + 1);
				goto done;
			}
			new_cmap_windows[0] = tmp->w;  /* add to front */
			for(i = 0; i < number_cmap_windows; i++) {   /* append rest */
				new_cmap_windows[i + 1] = cmap_windows[i];
			}
			XFree(cmap_windows);
			can_free_cmap_windows = true;  /* do not use XFree any more */
			cmap_windows = new_cmap_windows;
			number_cmap_windows++;
		}

		cwins = calloc(number_cmap_windows, sizeof(ColormapWindow *));
		if(cwins) {
			for(i = 0; i < number_cmap_windows; i++) {

				/*
				 * Copy any existing entries into new list.
				 */
				for(j = 0; j < tmp->cmaps.number_cwins; j++) {
					if(tmp->cmaps.cwins[j]->w == cmap_windows[i]) {
						cwins[i] = tmp->cmaps.cwins[j];
						cwins[i]->refcnt++;
						break;
					}
				}

				/*
				 * If the colormap window is not being pointed by
				 * some other applications colormap window list,
				 * create a new entry.
				 */
				if(j == tmp->cmaps.number_cwins) {
					if(XFindContext(dpy, cmap_windows[i], ColormapContext,
					                (XPointer *)&cwins[i]) == XCNOENT) {
						if((cwins[i] = CreateColormapWindow(cmap_windows[i],
						                                    tmp->cmaps.number_cwins == 0,
						                                    true)) == NULL) {
							int k;
							for(k = i + 1; k < number_cmap_windows; k++) {
								cmap_windows[k - 1] = cmap_windows[k];
							}
							i--;
							number_cmap_windows--;
						}
					}
					else {
						cwins[i]->refcnt++;
					}
				}
			}
		}
	}

	/* No else here, in case we bailed out of clause above.
	 */
	if(number_cmap_windows == 0) {

		number_cmap_windows = 1;

		cwins = malloc(sizeof(ColormapWindow *));
		if(XFindContext(dpy, tmp->w, ColormapContext, (XPointer *)&cwins[0]) ==
		                XCNOENT)
			cwins[0] = CreateColormapWindow(tmp->w,
			                                tmp->cmaps.number_cwins == 0, false);
		else {
			cwins[0]->refcnt++;
		}
	}

	if(tmp->cmaps.number_cwins) {
		free_cwins(tmp);
	}

	tmp->cmaps.cwins = cwins;
	tmp->cmaps.number_cwins = number_cmap_windows;
	if(number_cmap_windows > 1)
		tmp->cmaps.scoreboard =
		        calloc(1, ColormapsScoreboardLength(&tmp->cmaps));

	if(previnst) {
		InstallColormaps(PropertyNotify, NULL);
	}

done:
	if(cmap_windows) {
		if(can_free_cmap_windows) {
			free(cmap_windows);
		}
		else {
			XFree(cmap_windows);
		}
	}
}


/*
 * BumpWindowColormap - rotate our internal copy of WM_COLORMAP_WINDOWS.
 * This is the backend for f.colormap.
 *
 * Previously in functions.c
 */
void
BumpWindowColormap(TwmWindow *tmp, int inc)
{
	int i, j;
	bool previously_installed;
	ColormapWindow **cwins;

	if(!tmp) {
		return;
	}

	if(inc && tmp->cmaps.number_cwins > 0) {
		cwins = calloc(tmp->cmaps.number_cwins, sizeof(ColormapWindow *));
		if(cwins) {
			previously_installed = (Scr->cmapInfo.cmaps == &tmp->cmaps &&
			                        tmp->cmaps.number_cwins);
			if(previously_installed) {
				for(i = tmp->cmaps.number_cwins; i-- > 0;) {
					tmp->cmaps.cwins[i]->colormap->state = 0;
				}
			}

			for(i = 0; i < tmp->cmaps.number_cwins; i++) {
				j = i - inc;
				if(j >= tmp->cmaps.number_cwins) {
					j -= tmp->cmaps.number_cwins;
				}
				else if(j < 0) {
					j += tmp->cmaps.number_cwins;
				}
				cwins[j] = tmp->cmaps.cwins[i];
			}

			free(tmp->cmaps.cwins);

			tmp->cmaps.cwins = cwins;

			if(tmp->cmaps.number_cwins > 1)
				memset(tmp->cmaps.scoreboard, 0,
				       ColormapsScoreboardLength(&tmp->cmaps));

			if(previously_installed) {
				InstallColormaps(PropertyNotify, NULL);
			}
		}
	}
	else {
		FetchWmColormapWindows(tmp);
	}
	return;
}


/*
 * Handlers for creating and linkin in, as well as deleting, a StdCmap
 * for a given XStandardColormap.
 *
 * Previously in util.c
 */
void
InsertRGBColormap(Atom a, XStandardColormap *maps, int nmaps,
                  bool replace)
{
	StdCmap *sc = NULL;

	if(replace) {                       /* locate existing entry */
		for(sc = Scr->StdCmapInfo.head; sc; sc = sc->next) {
			if(sc->atom == a) {
				break;
			}
		}
	}

	if(!sc) {                           /* no existing, allocate new */
		sc = calloc(1, sizeof(StdCmap));
		if(!sc) {
			fprintf(stderr, "%s:  unable to allocate %lu bytes for StdCmap\n",
			        ProgramName, (unsigned long) sizeof(StdCmap));
			return;
		}
	}

	if(replace) {                       /* just update contents */
		if(sc->maps) {
			XFree(maps);
		}
		if(sc == Scr->StdCmapInfo.mru) {
			Scr->StdCmapInfo.mru = NULL;
		}
	}
	else {                              /* else appending */
		sc->next = NULL;
		sc->atom = a;
		if(Scr->StdCmapInfo.tail) {
			Scr->StdCmapInfo.tail->next = sc;
		}
		else {
			Scr->StdCmapInfo.head = sc;
		}
		Scr->StdCmapInfo.tail = sc;
	}
	sc->nmaps = nmaps;
	sc->maps = maps;

	return;
}


void
RemoveRGBColormap(Atom a)
{
	StdCmap *sc, *prev;

	prev = NULL;
	for(sc = Scr->StdCmapInfo.head; sc; sc = sc->next) {
		if(sc->atom == a) {
			break;
		}
		prev = sc;
	}
	if(sc) {                            /* found one */
		if(sc->maps) {
			XFree(sc->maps);
		}
		if(prev) {
			prev->next = sc->next;
		}
		if(Scr->StdCmapInfo.head == sc) {
			Scr->StdCmapInfo.head = sc->next;
		}
		if(Scr->StdCmapInfo.tail == sc) {
			Scr->StdCmapInfo.tail = prev;
		}
		if(Scr->StdCmapInfo.mru == sc) {
			Scr->StdCmapInfo.mru = NULL;
		}
	}
	return;
}


/*
 * Go through all the properties of the root window and setup
 * XStandardColormap's and our StdCmap's for any of them that are
 * actually RGB_COLORMAP types.  We're not actually _checking_ the types,
 * just letting XGetRGBColormaps() refuse to handle probably most of
 * them.  Called during startup.
 *
 * Previouly in util.c
 */
void
LocateStandardColormaps(void)
{
	Atom *atoms;
	int natoms;
	int i;

	atoms = XListProperties(dpy, Scr->Root, &natoms);
	for(i = 0; i < natoms; i++) {
		XStandardColormap *maps = NULL;
		int nmaps;

		if(XGetRGBColormaps(dpy, Scr->Root, &maps, &nmaps, atoms[i])) {
			/* if got one, then append to current list */
			InsertRGBColormap(atoms[i], maps, nmaps, false);
		}
	}
	if(atoms) {
		XFree(atoms);
	}
	return;
}


/*
 * Clear out and free TwmWindow.cmaps (struct Colormaps) bits for a window.
 *
 * Previously in events.c
 */
void
free_cwins(TwmWindow *tmp)
{
	int i;
	TwmColormap *cmap;

	if(tmp->cmaps.number_cwins) {
		for(i = 0; i < tmp->cmaps.number_cwins; i++) {
			if(--tmp->cmaps.cwins[i]->refcnt == 0) {
				cmap = tmp->cmaps.cwins[i]->colormap;
				if(--cmap->refcnt == 0) {
					XDeleteContext(dpy, cmap->c, ColormapContext);
					free(cmap);
				}
				XDeleteContext(dpy, tmp->cmaps.cwins[i]->w, ColormapContext);
				free(tmp->cmaps.cwins[i]);
			}
		}
		free(tmp->cmaps.cwins);
		if(tmp->cmaps.number_cwins > 1) {
			free(tmp->cmaps.scoreboard);
			tmp->cmaps.scoreboard = NULL;
		}
		tmp->cmaps.number_cwins = 0;
	}
}
