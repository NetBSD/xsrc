#include <stdio.h>
#include "twm.h"
#include "screen.h"
#ifdef VMS
#include <ctype.h>
#include <string.h>
#include <decw$include/Xos.h>
#include <decw$include/Xatom.h>
#include <X11Xmu/CharSet.h>
#include <decw$include/Xresource.h>
#else
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xresource.h>
#endif

#define PROTOCOLS_COUNT 5

Atom _XA_WIN_WORKSPACE;
Atom _XA_WIN_STATE;
static Atom _XA_WIN_CLIENT_LIST;


void InitGnome (void) {
  long curws = 0;
  virtualScreen *vs;
  Atom _XA_WIN_SUPPORTING_WM_CHECK, _XA_WIN_PROTOCOLS,
    _XA_WIN_PROTOCOLS_LIST[PROTOCOLS_COUNT], _XA_WIN_DESKTOP_BUTTON_PROXY;
  XWindowAttributes winattrs;
  unsigned long eventMask;

  XGetWindowAttributes(dpy, Scr->Root, &winattrs);
  eventMask = winattrs.your_event_mask;
  XSelectInput(dpy, Scr->Root, eventMask & ~PropertyChangeMask);
	
  _XA_WIN_SUPPORTING_WM_CHECK  = XInternAtom (dpy, "_WIN_SUPPORTING_WM_CHECK", False);
  _XA_WIN_DESKTOP_BUTTON_PROXY = XInternAtom (dpy, "_WIN_DESKTOP_BUTTON_PROXY", False);

  for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
    XChangeProperty (dpy, vs->wsw->w, _XA_WIN_SUPPORTING_WM_CHECK, XA_CARDINAL, 32, 
		    PropModeReplace,  (unsigned char *) &(vs->wsw->w), 1);

    XChangeProperty (dpy, Scr->Root,  _XA_WIN_SUPPORTING_WM_CHECK, XA_CARDINAL, 32, 
		    PropModeReplace,  (unsigned char *) &(vs->wsw->w), 1);

    XChangeProperty (dpy, vs->wsw->w, _XA_WIN_DESKTOP_BUTTON_PROXY, XA_CARDINAL, 32, 
		    PropModeReplace,  (unsigned char *) &(vs->wsw->w), 1);

    XChangeProperty (dpy, Scr->Root,  _XA_WIN_DESKTOP_BUTTON_PROXY, XA_CARDINAL, 32, 
		    PropModeReplace,  (unsigned char *) &(vs->wsw->w), 1);
  }
  _XA_WIN_PROTOCOLS = XInternAtom (dpy, "_WIN_PROTOCOLS", False);
  _XA_WIN_PROTOCOLS_LIST[0] = XInternAtom(dpy, "_WIN_WORKSPACE", False);
  _XA_WIN_PROTOCOLS_LIST[1] = XInternAtom(dpy, "_WIN_WORKSPACE_COUNT", False);
  _XA_WIN_PROTOCOLS_LIST[2] = XInternAtom(dpy, "_WIN_WORKSPACE_NAMES", False);
  _XA_WIN_PROTOCOLS_LIST[3] = XInternAtom(dpy, "_WIN_CLIENT_LIST", False);
  _XA_WIN_PROTOCOLS_LIST[4] = XInternAtom(dpy, "_WIN_STATE", False);
  _XA_WIN_WORKSPACE = _XA_WIN_PROTOCOLS_LIST[0];
  _XA_WIN_CLIENT_LIST = _XA_WIN_PROTOCOLS_LIST[3];
  _XA_WIN_STATE = _XA_WIN_PROTOCOLS_LIST[4];
	
  XChangeProperty (dpy, Scr->Root, _XA_WIN_PROTOCOLS, XA_ATOM, 32, 
		   PropModeReplace, (unsigned char *) _XA_WIN_PROTOCOLS_LIST,
		   PROTOCOLS_COUNT);

  XChangeProperty (dpy, Scr->Root, _XA_WIN_PROTOCOLS_LIST[1], XA_CARDINAL, 32,
		   PropModeReplace, (unsigned char *) &(Scr->workSpaceMgr.count), 1);

  if (!captive)
    XChangeProperty (dpy, Scr->Root, _XA_WIN_PROTOCOLS_LIST[0], XA_CARDINAL, 32,
		   PropModeReplace, (unsigned char *) &curws, 1);

  XSelectInput (dpy, Scr->Root, eventMask);

  Scr->gnomedata = (GnomeData*) malloc (sizeof (GnomeData));
  Scr->gnomedata->ws = malloc (sizeof (Window));
  if (Scr->gnomedata->ws) {
    Scr->gnomedata->wsSize = 1;
    Scr->gnomedata->numWins = 0;
  } else {
    Scr->gnomedata->wsSize = 0;
    Scr->gnomedata->numWins = 0;
  }
  XChangeProperty (dpy, Scr->Root, _XA_WIN_CLIENT_LIST, XA_CARDINAL, 32, 
		   PropModeReplace, (unsigned char *)Scr->gnomedata->ws,
		   Scr->gnomedata->numWins);
}

void GnomeAddClientWindow (TwmWindow *new_win) {
  XWindowAttributes winattrs;
  unsigned long eventMask;

  if (Scr->gnomedata->wsSize == 0) return;
  if ((!LookInList (Scr->IconMgrNoShow, new_win->full_name, &new_win->class)) && 
      (new_win->w != Scr->workSpaceMgr.occupyWindow->w) && 
      (!new_win->iconmgr)) {
    Scr->gnomedata->numWins++;
    if (Scr->gnomedata->numWins > Scr->gnomedata->wsSize) {
      Scr->gnomedata->wsSize *= 2;
      Scr->gnomedata->ws = realloc(Scr->gnomedata->ws, sizeof(Window) * Scr->gnomedata->wsSize);
    }
    if (Scr->gnomedata->ws) Scr->gnomedata->ws [Scr->gnomedata->numWins - 1] = new_win->w;
    else {
      Scr->gnomedata->numWins--;
      fprintf (stderr, "Unable to allocate memory for GNOME client list.\n");
      return;
    }
    XGetWindowAttributes (dpy, Scr->Root, &winattrs);
    eventMask = winattrs.your_event_mask;
    XSelectInput(dpy, Scr->Root, eventMask & ~PropertyChangeMask);		
    XChangeProperty(dpy, Scr->Root, _XA_WIN_CLIENT_LIST, XA_CARDINAL, 32, 
		    PropModeReplace, (unsigned char *)Scr->gnomedata->ws,
		    Scr->gnomedata->numWins);
    XSelectInput(dpy, Scr->Root, eventMask);
  }
}

void GnomeDeleteClientWindow (TwmWindow *new_win) {
  XWindowAttributes winattrs;
  unsigned long eventMask;
  int i;

  if (Scr->gnomedata->wsSize == 0) return;
  if ((!LookInList(Scr->IconMgrNoShow, new_win->full_name, &new_win->class)) && 
      (new_win->w != Scr->workSpaceMgr.occupyWindow->w) && 
      (!new_win->iconmgr)) {
    for (i = 0; i < Scr->gnomedata->numWins; i++){
      if(Scr->gnomedata->ws[i] == new_win->w){
	Scr->gnomedata->numWins--;
	Scr->gnomedata->ws[i] = Scr->gnomedata->ws[Scr->gnomedata->numWins];
	Scr->gnomedata->ws[Scr->gnomedata->numWins] = 0;
	if(Scr->gnomedata->numWins &&
	   (Scr->gnomedata->numWins * 3) < Scr->gnomedata->wsSize) {
	  Scr->gnomedata->wsSize /= 2;
	  Scr->gnomedata->ws = realloc (Scr->gnomedata->ws,
					sizeof (Window) * Scr->gnomedata->wsSize);
	  /* memory shrinking, shouldn't have problems */
	}
	break;
      }
    }
    XGetWindowAttributes (dpy, Scr->Root, &winattrs);
    eventMask = winattrs.your_event_mask;
    XSelectInput(dpy, Scr->Root, eventMask & ~PropertyChangeMask);		
    XChangeProperty(dpy, Scr->Root, _XA_WIN_CLIENT_LIST, XA_CARDINAL, 32, 
		    PropModeReplace, (unsigned char *)Scr->gnomedata->ws,
		    Scr->gnomedata->numWins);
    XSelectInput(dpy, Scr->Root, eventMask);
  }
}
