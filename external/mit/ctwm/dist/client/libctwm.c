/*
 * Copyright 1992 Claude Lecommandeur.
 */
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include "ctwm.h"
#include "ctwm_client.h"

Atom _XA_WM_OCCUPATION;
Atom _XA_WM_CURRENTWORKSPACE;
Atom _XA_WM_WORKSPACESLIST;

/* Note that this doesn't assure that ctwm is really running,
   i should set up a communication via flipping a property */

bool CtwmIsRunning(Display *display, int scrnum)
{
	unsigned char       *prop;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;

	_XA_WM_WORKSPACESLIST = XInternAtom(display, "WM_WORKSPACESLIST", True);
	if(_XA_WM_WORKSPACESLIST == None) {
		return false;
	}
	if(XGetWindowProperty(display, RootWindow(display, scrnum),
	                      _XA_WM_WORKSPACESLIST, 0L, 512,
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

char **CtwmListWorkspaces(Display *display, int scrnum)
{
	unsigned char       *prop;
	char                *p;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;
	char                **ret;
	int                 count;
	int                 i;
	unsigned long       l;

	_XA_WM_WORKSPACESLIST = XInternAtom(display, "_WIN_WORKSPACE_NAMES", True);

	if(XGetWindowProperty(display, RootWindow(display, scrnum),
	                      _XA_WM_WORKSPACESLIST, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return 0;
	}
	if(len == 0) {
		return 0;
	}

	count = 0;
	p = (char *)prop;
	l = 0;
	while(l < len) {
		l += strlen(p) + 1;
		p += strlen(p) + 1;
		count++;
	}
	ret = calloc(count + 1, sizeof(char *));

	p = (char *)prop;
	l = 0;
	i = 0;
	while(l < len) {
		ret [i++] = p;
		l += strlen(p) + 1;
		p += strlen(p) + 1;
	}
	ret [i] = NULL;
	XFree(prop);
	return (ret);
}

char *CtwmCurrentWorkspace(Display *display, int scrnum)
{
	unsigned char       *prop;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;

	_XA_WM_CURRENTWORKSPACE = XInternAtom(display, "WM_CURRENTWORKSPACE", True);
	if(_XA_WM_CURRENTWORKSPACE == None) {
		return NULL;
	}

	if(XGetWindowProperty(display, RootWindow(display, scrnum),
	                      _XA_WM_CURRENTWORKSPACE, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return NULL;
	}
	if(len == 0) {
		return NULL;
	}
	return ((char *) prop);
}

int CtwmChangeWorkspace(Display *display, int scrnum, char     *workspace)
{
	_XA_WM_CURRENTWORKSPACE = XInternAtom(display, "WM_CURRENTWORKSPACE", True);
	if(_XA_WM_CURRENTWORKSPACE == None) {
		return (0);
	}

	XChangeProperty(display, RootWindow(display, scrnum), _XA_WM_CURRENTWORKSPACE,
	                XA_STRING, 8,
	                PropModeReplace, (unsigned char *) workspace, strlen(workspace));
	XFlush(display);
	return (1);
}

char **CtwmCurrentOccupation(Display *display, Window window)
{
	unsigned char       *prop;
	char                *p;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;
	int                 count, i;
	unsigned long       l;
	char                **ret;

	_XA_WM_OCCUPATION = XInternAtom(display, "WM_OCCUPATION", True);
	if(_XA_WM_OCCUPATION == None) {
		return NULL;
	}

	if(XGetWindowProperty(display, window, _XA_WM_OCCUPATION, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return NULL;
	}
	if(len == 0) {
		return NULL;
	}

	count = 0;
	p = (char *)prop;
	l = 0;
	while(l < len) {
		l += strlen(p) + 1;
		p += strlen(p) + 1;
		count++;
	}
	ret = calloc(count + 1, sizeof(char *));

	p = (char *)prop;
	l = 0;
	i = 0;
	while(l < len) {
		ret [i++] = p;
		l += strlen(p) + 1;
		p += strlen(p) + 1;
	}
	ret [i] = NULL;
	XFree(prop);
	return ret;
}

int CtwmSetOccupation(Display *display, Window window, char **occupation)
{
	int         len;
	char        **occ;
	char        *occup, *o;

	_XA_WM_OCCUPATION = XInternAtom(display, "WM_OCCUPATION", True);
	if(_XA_WM_OCCUPATION == None) {
		return (0);
	}

	occ = occupation;
	len = 0;
	while(*occ++) {
		len += strlen(*occupation) + 1;
	}
	occup = calloc(len, sizeof(char));

	o = occup;
	while(*occupation) {
		strcpy(o, *occupation);
		o += strlen(*occupation) + 1;
		occupation++;
	}
	XChangeProperty(display, window, _XA_WM_OCCUPATION, XA_STRING, 8,
	                PropModeReplace, (unsigned char *) occup, len - 1);
	XFlush(display);
	free(occup);
	return (1);
}

int CtwmAddToCurrentWorkspace(Display *display, Window window)
{
	unsigned char       *prop;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;
	XWindowAttributes   attr;
	unsigned char       *currentw;

	if(! XGetWindowAttributes(display, window, &attr)) {
		return (0);
	}

	_XA_WM_CURRENTWORKSPACE = XInternAtom(display, "WM_CURRENTWORKSPACE", True);
	if(_XA_WM_CURRENTWORKSPACE == None) {
		return (0);
	}

	if(XGetWindowProperty(display, attr.root, _XA_WM_CURRENTWORKSPACE, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &currentw) != Success) {
		return (0);
	}
	if(len == 0) {
		return (0);
	}

	_XA_WM_OCCUPATION = XInternAtom(display, "WM_OCCUPATION", True);
	if(_XA_WM_OCCUPATION == None) {
		return (0);
	}

	if(XGetWindowProperty(display, window, _XA_WM_OCCUPATION, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return (0);
	}
	if(len == 0) {
		return (0);
	}

	strcpy((char *)prop + len, (char *)currentw);
	XChangeProperty(display, window, _XA_WM_OCCUPATION, XA_STRING, 8,
	                PropModeReplace,
	                prop, (int) len + strlen((char *)currentw));
	XFree(prop);
	XFree(currentw);
	XFlush(display);
	return (1);
}
