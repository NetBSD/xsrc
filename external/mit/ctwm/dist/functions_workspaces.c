/*
 * Functions related to window occupation and workspaces.  Not the
 * workspace manager itself; that's off with the icon managers.
 */

#include "ctwm.h"

#include "functions_internal.h"
#include "screen.h"
#include "occupation.h"
#include "workspace_utils.h"



/*
 * Setting occupation on a specific window.
 */
DFHANDLER(occupy)
{
	Occupy(tmp_win);
}

DFHANDLER(occupyall)
{
	OccupyAll(tmp_win);
}


/*
 * Selecting a window and passing a specific workspace as the function
 * arg.
 */
DFHANDLER(addtoworkspace)
{
	AddToWorkSpace(action, tmp_win);
}

DFHANDLER(removefromworkspace)
{
	RemoveFromWorkSpace(action, tmp_win);
}

DFHANDLER(toggleoccupation)
{
	ToggleOccupation(action, tmp_win);
}


/*
 * Pushing a window away from / pulling it to "here".
 */
DFHANDLER(vanish)
{
	WMgrRemoveFromCurrentWorkSpace(Scr->currentvs, tmp_win);
}

DFHANDLER(warphere)
{
	WMgrAddToCurrentWorkSpaceAndWarp(Scr->currentvs, action);
}


/*
 * Pushing a window away somewhere and potentially following it.
 */
DFHANDLER(movetonextworkspace)
{
	MoveToNextWorkSpace(Scr->currentvs, tmp_win);
}

DFHANDLER(movetoprevworkspace)
{
	MoveToPrevWorkSpace(Scr->currentvs, tmp_win);
}

DFHANDLER(movetonextworkspaceandfollow)
{
	MoveToNextWorkSpaceAndFollow(Scr->currentvs, tmp_win);
}

DFHANDLER(movetoprevworkspaceandfollow)
{
	MoveToPrevWorkSpaceAndFollow(Scr->currentvs, tmp_win);
}



/*
 * Switching to other workspaces.
 */
DFHANDLER(gotoworkspace)
{
	/*
	 * n.b.: referenced in the Developer Manual in doc/devman/; if you
	 * make any changes here be sure to tweak that if necessary.
	 */
	GotoWorkSpaceByName(Scr->currentvs, action);
}

DFHANDLER(prevworkspace)
{
	GotoPrevWorkSpace(Scr->currentvs);
}

DFHANDLER(nextworkspace)
{
	GotoNextWorkSpace(Scr->currentvs);
}

DFHANDLER(rightworkspace)
{
	GotoRightWorkSpace(Scr->currentvs);
}

DFHANDLER(leftworkspace)
{
	GotoLeftWorkSpace(Scr->currentvs);
}

DFHANDLER(upworkspace)
{
	GotoUpWorkSpace(Scr->currentvs);
}

DFHANDLER(downworkspace)
{
	GotoDownWorkSpace(Scr->currentvs);
}
