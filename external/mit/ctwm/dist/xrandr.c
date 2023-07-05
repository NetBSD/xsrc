/*
 * Copyright notice...
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>

#include "r_area_list.h"
#include "r_area.h"
#include "r_layout.h"
#include "xrandr.h"


/**
 * Use XRANDR to figure out how our monitors are laid out.
 */
RLayout *
XrandrNewLayout(Display *disp, Window rootw)
{
	int i_nmonitors = 0;
	XRRMonitorInfo *ps_monitors;
	char **monitor_names;
	RAreaList *areas;
	int evt_base, err_base, ver_maj, ver_min;
	// XXX *_base and ver_* should move more globally if we start doing
	// randr stuff anywhere else.

	// If the server doesn't talk RANDR, we have nothing to do.
	if(XRRQueryExtension(disp, &evt_base, &err_base) != True) {
		// No RANDR
#ifdef DEBUG
		fprintf(stderr, "No RANDR on the server.\n");
#endif
		return NULL;
	}

	// XRRGetMonitors() wraps the RRGetMonitors request, which requires
	// 1.5.
	if(XRRQueryVersion(disp, &ver_maj, &ver_min) == 0) {
		// Couldn't get the version
#ifdef DEBUG
		fprintf(stderr, "Couldn't get server RANDR version.\n");
#endif
		return NULL;
	}
	if(ver_maj < 1 || (ver_maj == 1 && ver_min < 5)) {
#ifdef DEBUG
		fprintf(stderr, "Server has RANDR %d.%d, we need 1.5+.\n",
		        ver_maj, ver_min);
#endif
		return NULL;
	}

	// RANDR 1.5 function to get info about 'em.
	ps_monitors = XRRGetMonitors(disp, rootw, 1, &i_nmonitors);
	if(ps_monitors == NULL || i_nmonitors == 0) {
		fprintf(stderr, "XRRGetMonitors failed\n");
		return NULL;
	}

	// Useful note: vtwm also apparently has RANDR support.  It uses
	// XRRGetScreenResources() and looping XRRGetCrtcInfo() to load info
	// about the screen, and its conditionals suggest that's RANDR 1.2.
	// Look into that if we decide to worry about earlier versions.

	// Add space for all their names (plus trailing NULL)
	monitor_names = calloc((i_nmonitors + 1), sizeof(char *));
	if(monitor_names == NULL) {
		abort();
	}

	// Add each and its name into an RAreaList
	areas = RAreaListNew(i_nmonitors, NULL);
	for(int i = 0; i < i_nmonitors; i++) {
		RArea cur_area = RAreaNew(ps_monitors[i].x,
		                          ps_monitors[i].y,
		                          ps_monitors[i].width,
		                          ps_monitors[i].height);

		char *name = XGetAtomName(disp, ps_monitors[i].name);
#ifdef DEBUG
		fprintf(stderr, "NEW area: %s%s",
		        name != NULL ? name : "",
		        name != NULL ? ":" : "");
		RAreaPrint(&cur_area);
		fprintf(stderr, "\n");
#endif
		if(name != NULL) {
			monitor_names[i] = strdup(name);
			XFree(name);
		}
		else {
			monitor_names[i] = strdup("");
		}

		RAreaListAdd(areas, &cur_area);
	}

	XRRFreeMonitors(ps_monitors);

	// Build up an RLayout of those areas and their names, and hand it
	// back.
	return RLayoutSetMonitorsNames(RLayoutNew(areas), monitor_names);
}
