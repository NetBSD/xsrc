/*
 * Routines using in setting up the config for workspace stuff.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "image.h"
#include "list.h"
#include "occupation.h"
#include "screen.h"
#include "util.h"
#include "workspace_config.h"
#include "workspace_utils.h"


// Temp; x-ref desc in workspace_utils
extern bool useBackgroundInfo;


/*
 * Create a workspace.  This is what gets called when parsing
 * WorkSpaces {} config file entries.
 *
 * WorkSpaces {
 *  "One" { name background foreground backback backfore "backpix.jpg" }
 *  #         |      |           |        |       |            |
 *  #     WS name    |      Button Text   |   Map/Root FG      |
 *  #            Button BG             Map BG            Map/Root BG img
 * }
 */
void
AddWorkSpace(const char *name, const char *background, const char *foreground,
             const char *backback, const char *backfore, const char *backpix)
{
	WorkSpace *ws;

	/* XXX Shouldn't just silently return if we're already at max...  */
	if(Scr->workSpaceMgr.count == MAXWORKSPACE) {
		return;
	}

	/* Init.  Label can change over time, but starts the same as name. */
	ws = calloc(1, sizeof(WorkSpace));
	ws->name  = strdup(name);
	ws->label = strdup(name);
	ws->number = Scr->workSpaceMgr.count++;

	/* We're a new entry on the "everything" list */
	fullOccupation |= (1 << ws->number);


	/*
	 * FB/BG colors for the button state may be specified, or fallback to
	 * the icon manager's if not.
	 */
	if(background == NULL) {
		ws->cp.back = Scr->IconManagerC.back;
	}
	else {
		GetColor(Scr->Monochrome, &(ws->cp.back), background);
	}

	if(foreground == NULL) {
		ws->cp.fore = Scr->IconManagerC.fore;
	}
	else {
		GetColor(Scr->Monochrome, &(ws->cp.fore), foreground);
	}

	/* Shadows for 3d buttons derived from that */
#ifdef COLOR_BLIND_USER
	ws->cp.shadc = Scr->White;
	ws->cp.shadd = Scr->Black;
#else
	if(!Scr->BeNiceToColormap) {
		GetShadeColors(&ws->cp);
	}
#endif


	/*
	 * Map-state fb/bg color, as well as root win background.
	 */
	if(backback == NULL) {
		GetColor(Scr->Monochrome, &(ws->backcp.back), "Black");
	}
	else {
		GetColor(Scr->Monochrome, &(ws->backcp.back), backback);
		useBackgroundInfo = true;
	}

	if(backfore == NULL) {
		GetColor(Scr->Monochrome, &(ws->backcp.fore), "White");
	}
	else {
		GetColor(Scr->Monochrome, &(ws->backcp.fore), backfore);
		useBackgroundInfo = true;
	}


	/* Maybe there's an image to stick on the root as well */
	ws->image = GetImage(backpix, ws->backcp);
	if(ws->image != NULL) {
		useBackgroundInfo = true;
	}


	/* Put ourselves on the end of the workspace list */
	if(Scr->workSpaceMgr.workSpaceList == NULL) {
		Scr->workSpaceMgr.workSpaceList = ws;
	}
	else {
		WorkSpace *wstmp = Scr->workSpaceMgr.workSpaceList;
		while(wstmp->next != NULL) {
			wstmp = wstmp->next;
		}
		wstmp->next = ws;
	}

	/* There's at least one defined WS now */
	Scr->workSpaceManagerActive = true;

	return;
}



/*
 * MapWindowCurrentWorkSpace {} parsing
 */
void
WMapCreateCurrentBackGround(const char *border,
                            const char *background, const char *foreground,
                            const char *pixmap)
{
	Image *image;
	WorkSpaceMgr *ws = &Scr->workSpaceMgr;

	ws->curBorderColor = Scr->Black;
	ws->curColors.back = Scr->White;
	ws->curColors.fore = Scr->Black;
	ws->curImage       = NULL;

	if(border == NULL) {
		return;
	}
	GetColor(Scr->Monochrome, &ws->curBorderColor, border);
	if(background == NULL) {
		return;
	}
	ws->curPaint = true;
	GetColor(Scr->Monochrome, &ws->curColors.back, background);
	if(foreground == NULL) {
		return;
	}
	GetColor(Scr->Monochrome, &ws->curColors.fore, foreground);

	if(pixmap == NULL) {
		return;
	}
	if((image = GetImage(pixmap, Scr->workSpaceMgr.curColors)) == NULL) {
		fprintf(stderr, "Can't find pixmap %s\n", pixmap);
		return;
	}
	ws->curImage = image;
}



/*
 * MapWindowDefaultWorkSpace {} parsing
 */
void
WMapCreateDefaultBackGround(const char *border,
                            const char *background, const char *foreground,
                            const char *pixmap)
{
	Image *image;
	WorkSpaceMgr *ws = &Scr->workSpaceMgr;

	ws->defBorderColor = Scr->Black;
	ws->defColors.back = Scr->White;
	ws->defColors.fore = Scr->Black;
	ws->defImage       = NULL;

	if(border == NULL) {
		return;
	}
	GetColor(Scr->Monochrome, &ws->defBorderColor, border);
	if(background == NULL) {
		return;
	}
	GetColor(Scr->Monochrome, &ws->defColors.back, background);
	if(foreground == NULL) {
		return;
	}
	GetColor(Scr->Monochrome, &ws->defColors.fore, foreground);
	if(pixmap == NULL) {
		return;
	}
	if((image = GetImage(pixmap, ws->defColors)) == NULL) {
		return;
	}
	ws->defImage = image;
}
