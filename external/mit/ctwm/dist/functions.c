/*
 * Dispatcher for our f.whatever functions
 */


#include "ctwm.h"

#include <stdio.h>

#include "events.h"
#include "functions.h"
#include "functions_defs.h"
#include "functions_deferral.h"  // Generated deferral table
#include "functions_internal.h"
#include "screen.h"


static DFHANDLER(nop);
static DFHANDLER(separator);
static DFHANDLER(title);
static DFHANDLER(deltastop);
static DFHANDLER(function);

/*
 * The generated dispatch table.  Have to do this after the preceeding
 * prototypes for the handers in this file, since those funcs are ref'd
 * in this table.
 */
#include "functions_dispatch_execution.h"


/*
 * Various functions can be executed "from the root" (which generally
 * means "from a menu"), but apply to a specific window (e.g., f.move,
 * f.identify, etc).  You obviously can't be selecting it from a menu and
 * pointing at the window to target at the same time, so we have to
 * 2-step those calls.  This happens via the DeferExecution() call in the
 * implementations of those functions, which stashes the "in progress"
 * function in RootFunction.  The HandleButtonPress() event handler will
 * later notice that and loop events back around into ExecuteFunction()
 * again to pick up where it left off.
 *
 * (a more descriptive name might be in order)
 */
int RootFunction = 0;


/*
 * Track whether a window gets moved by move operations: used for
 * f.deltastop handling.
 */
bool WindowMoved = false;

/*
 * Whether the cursor needs to be reset from a way we've altered it in
 * the process of running functions.  This is used to determine whether
 * we're ungrabbing the pointer to reset back from setting the WaitCursor
 * early on in the execution process.  X-ref the XXX comment on that;
 * it's unclear as to whether we should even be doing this anymore, but
 * since we are, we use a global to ease tracking whether we need to
 * unset it.  There are cases deeper down in function handling that may
 * do their own fudgery and want the pointer left alone after they
 * return.
 */
bool func_reset_cursor;

/*
 * Time of various actions: used in ConstrainedMoveTime related bits in
 * some window moving/resizing.
 */
Time last_time = 0;


static bool EF_main(EF_FULLPROTO);

static bool DeferExecution(int context, int func, Cursor cursor);
static bool should_defer(int func);
static Cursor defer_cursor(int func);
static Cursor NeedToDefer(MenuRoot *root);


/***********************************************************************
 *
 *  Procedure:
 *      ExecuteFunction - execute a twm root function
 *
 *  Inputs:
 *      func    - the function to execute
 *      action  - the menu action to execute
 *      w       - the window to execute this function on
 *      tmp_win - the twm window structure
 *      event   - the event that caused the function
 *      context - the context in which the button was pressed
 *      pulldown- flag indicating execution from pull down menu
 *
 ***********************************************************************
 */
void
ExecuteFunction(EF_FULLPROTO)
{
	EF_main(EF_ARGS);
}

/*
 * Main ExecuteFunction body; returns true if we should continue a
 * f.function's progress, false if we should stop.
 *
 * This is separate because only the recursive calls in f.function
 * handling care about that return.  The only possible way to get to a
 * false return is via f.deltastop triggering.  We can't do it just with
 * a global, since f.function can at least in theory happen recursively;
 * I don't know how well it would actually work, but it has a chance.
 */
static bool
EF_main(EF_FULLPROTO)
{
	/* This should always start out clear when we come in here */
	RootFunction = 0;

	/* Early escape for cutting out of things */
	if(Cancel) {
		/*
		 * Strictly, this could probably be false, since if it's set it
		 * would mean it'll just happen again when we iterate back
		 * through for the next action.  Once set, it only gets unset in
		 * the ButtonRelease handler, which I don't think would ever get
		 * called in between pieces of a f.function call.  But I can't be
		 * sure, so just go ahead and return true, and we'll eat a few
		 * extra loops of function calls and insta-returns if it happens.
		 */
		return true;
	}


	/*
	 * More early escapes; some "functions" don't actually do anything
	 * when executed, and exist for magical purposes elsewhere.  So just
	 * skip out early if we try running them.
	 */
	switch(func) {
		case F_NOP:
		case F_SEPARATOR:
		case F_TITLE:
			return true;

		default:
			; /* FALLTHRU */
	}


	/*
	 * Is this a function that needs some deferring?  If so, go ahead and
	 * do that.  Note that this specifically doesn't handle the special
	 * case of f.function; it has to do its own checking for whether
	 * there's something to defer.
	 */
	if(should_defer(func)) {
		/* Figure the cursor */
		Cursor dc = defer_cursor(func);
		if(dc == None) {
			dc = Scr->SelectCursor;
		}

		/* And defer (if we're in a context that needs to) */
		if(DeferExecution(context, func, dc)) {
			return true;
		}
	}


	/*
	 * For most functions with a few exceptions, grab the pointer.
	 *
	 * This is actually not a grab so much to take control of the
	 * pointer, as to set the cursor.  Apparently, xlib doesn't
	 * distinguish the two.  The functions that need it in a "take
	 * control" sense (like the move and resize bits) should all be doing
	 * their own explicit grabs to handle that.
	 *
	 * XXX I have no idea why there's the exclusion list.  Apart from
	 * adding 1 or 2 functions, this code comes verbatim from twm, which
	 * has no history or documentation as to why it's happening.
	 *
	 * XXX I'm not sure this is even worth doing anymore.  The point of
	 * the WaitCursor is to let the user know "yeah, I'm working on it",
	 * during operations that may take a while.  On 1985 hardware, that
	 * would be "almost anything you do".  But in the 21st century, what
	 * functions could fall into that category, and need to give some
	 * user feedback before either finishing or doing something that
	 * gives other user feedback anyway?
	 */
	func_reset_cursor = false;
	switch(func) {
		case F_UPICONMGR:
		case F_LEFTICONMGR:
		case F_RIGHTICONMGR:
		case F_DOWNICONMGR:
		case F_FORWICONMGR:
		case F_BACKICONMGR:
		case F_NEXTICONMGR:
		case F_PREVICONMGR:
		case F_NOP:
		case F_TITLE:
		case F_DELTASTOP:
		case F_RAISELOWER:
		case F_WARPTOSCREEN:
		case F_WARPTO:
		case F_WARPRING:
		case F_WARPTOICONMGR:
		case F_COLORMAP:
		case F_ALTKEYMAP:
		case F_ALTCONTEXT:
			break;

		default: {
			XGrabPointer(dpy, Scr->Root, True,
			             ButtonPressMask | ButtonReleaseMask,
			             GrabModeAsync, GrabModeAsync,
			             Scr->Root, Scr->WaitCursor, CurrentTime);
			func_reset_cursor = true;
			break;
		}
	}


	/*
	 * Main dispatching/executing.
	 *
	 * _Most_ f.things are dispatched to individual handler functions,
	 * but we keep the magic related to f.function/f.deltastop out here
	 * to free the inner bits from having to care about the magic
	 * returns.
	 */
	switch(func) {
		case F_DELTASTOP:
			if(WindowMoved) {
				/*
				 * If we're returning false here, it's because we were in
				 * the midst of a f.function, and we should stop.  That
				 * means when we return from here it'll be into the false
				 * case in the F_FUNCTION handler below, which will break
				 * right out and fall through to the end of this
				 * function, which will do the post-function cleanup
				 * bits.  That means we don't need to try and break out
				 * to them here, we can just return straight off.
				 */
				return false;
			}
			break;

		case F_FUNCTION: {
			MenuRoot *mroot;
			MenuItem *mitem;
			Cursor curs;

			if((mroot = FindMenuRoot(action)) == NULL) {
				if(!action) {
					action = "undef";
				}
				fprintf(stderr, "%s: couldn't find function \"%s\"\n",
				        ProgramName, (char *)action);
				return true;
			}

			if((curs = NeedToDefer(mroot)) != None
			                && DeferExecution(context, func, curs)) {
				return true;
			}
			else {
				for(mitem = mroot->first; mitem != NULL; mitem = mitem->next) {
					bool r = EF_main(mitem->func, mitem->action, w,
					                 tmp_win, eventp, context, pulldown);
					if(r == false) {
						/* pebl FIXME: the focus should be updated here,
						 or the function would operate on the same window */
						break;
					}
				}
			}

			break;
		}


		/*
		 * Everything else is programmatically dispatched.
		 */
		default: {
			if(func >= 0 && func < num_f_dis && func_dispatch[func] != NULL) {
				(*func_dispatch[func])(EF_ARGS);
				break;
			}

			/*
			 * Getting here means somehow it wasn't in the dispatch
			 * table, which shouldn't be possible without a big bug
			 * somewhere...
			 */
			fprintf(stderr, "Internal error: no handler for function %d\n",
			        func);
			break;
		}
	}



	/*
	 * Release the pointer.  This should mostly mean actually "reset
	 * cursor", and be the complementary op to setting the cursor earlier
	 * up top.
	 *
	 * ButtonPressed == -1 means that we didn't get here via some sort of
	 * mouse clickery.  If we did, then we presume that has some
	 * ownership of the pointer we don't want to relinquish yet.  And we
	 * don't have to, as the ButtonRelease handler will take care of
	 * things when it fires anyway.
	 *
	 * This has a similar XXX to the cursor setting earlier, as to
	 * whether it ought to exist.
	 */
	if(func_reset_cursor && ButtonPressed == -1) {
		XUngrabPointer(dpy, CurrentTime);
		func_reset_cursor = false;
	}

	return true;
}



/*
 * Implementation of function deferral
 */

/*
 * Setting a last cursor and re-grabbing to it.  This is used in the
 * AddWindow() process.  It might grab the mouse and re-set the
 * cursor away from us, and so it needs a way to set it back.
 *
 * XXX This begs for renaming...
 */
static Cursor LastCursor;

void
ReGrab(void)
{
	XGrabPointer(dpy, Scr->Root, True,
	             ButtonPressMask | ButtonReleaseMask,
	             GrabModeAsync, GrabModeAsync,
	             Scr->Root, LastCursor, CurrentTime);
}


/*
 * Check to see if a function (implicitly, a window-targetting function)
 * is happening in a context away from an actual window, and if so stash
 * up info about what's in progress and return true to tell the caller to
 * end processing the function (for now).  X-ref comment on RootFunction
 * variable definition for details.
 *
 *  Inputs:
 *      context - the context in which the mouse button was pressed
 *      func    - the function to defer
 *      cursor  - the cursor to display while waiting
 */
static bool
DeferExecution(int context, int func, Cursor cursor)
{
	Window confine_to = Scr->Root;
#ifdef CAPTIVE
	if(func == F_ADOPTWINDOW) {
		confine_to = None;
	}
#endif

	if((context == C_ROOT) || (context == C_ALTERNATE)) {
		LastCursor = cursor;
		XGrabPointer(dpy,
		             Scr->Root,
		             True,
		             ButtonPressMask | ButtonReleaseMask,
		             GrabModeAsync,
		             GrabModeAsync,
		             confine_to,
		             cursor,
		             CurrentTime);
		RootFunction = func;

		return true;
	}

	return false;
}


/*
 * Various determinates of whether a function should be deferred if its
 * called in a general (rather than win-specific) context, and what
 * cursor should be used in the meantime.
 *
 * We define a big lookup array to do it.  We have to indirect through an
 * intermediate enum value instead of just the cursor since it isn't
 * available at compile time, and we can't just make it a pointer into
 * Scr since there are [potentially] multiple Scr's anyway.  And we need
 * an explicit unused DC_NONE value so our real values are all non-zero;
 * the ones we don't explicitly set get initialized to 0, which we can
 * then take as a flag saying "we don't defer this func".
 *
 * fdef_table in functions_deferral.h generated from functions_defs.list.
 */

static bool
should_defer(int func)
{
	/* Outside the table -> "No" */
	if(func < 0 || func >= fdef_table_max) {
		return false;
	}

	if(fdef_table[func] != DC_NONE) {
		return true;
	}
	return false;
}

static Cursor
defer_cursor(int func)
{
	/* Outside the table -> "No" */
	if(func < 0 || func >= fdef_table_max) {
		return None;
	}

	switch(fdef_table[func]) {
		case DC_SELECT:
			return Scr->SelectCursor;
		case DC_MOVE:
			return Scr->MoveCursor;
		case DC_DESTROY:
			return Scr->DestroyCursor;

		default:
			/* Is there a better choice? */
			return None;
	}

	/* NOTREACHED */
	return None;
}


/*
 * Checks each function in a user-defined Function list called via
 * f.function to see any of them need to be defered.  The Function config
 * action creates pseudo-menus to store the items in that call, so we
 * loop through the "items" in that "menu".  Try not to think about that
 * too much.
 *
 * This previously used a hardcoded list of functions to defer, which was
 * substantially smaller than the list it's currently checking.  It now
 * checks all the same functions that are themselves checked
 * individually, which is almost certainly how it should have always
 * worked anyway.
 */
static Cursor
NeedToDefer(MenuRoot *root)
{
	MenuItem *mitem;

	for(mitem = root->first; mitem != NULL; mitem = mitem->next) {
		if(should_defer(mitem->func)) {
			Cursor dc = defer_cursor(mitem->func);
			if(dc == None) {
				return Scr->SelectCursor;
			}
			return dc;
		}
	}
	return None;
}



/*
 * Faked up handlers for functions that shouldn't ever really get to
 * them.  These are handled in various hard-coded ways before we get to
 * automatic dispatch, so there shouldn't be any way these functions
 * actually get called.  But, just in case, return instead of dying.
 *
 * It's easier to just write these than to try and long-term parameterize
 * which we expect to exist.
 */

/* f.nop, f.title, f.separator really only exist to make lines in menus */
static
DFHANDLER(nop)
{
	fprintf(stderr, "%s(): Shouldn't get here.\n", __func__);
	return;
}
static
DFHANDLER(separator)
{
	fprintf(stderr, "%s(): Shouldn't get here.\n", __func__);
	return;
}
static
DFHANDLER(title)
{
	fprintf(stderr, "%s(): Shouldn't get here.\n", __func__);
	return;
}

/* f.deltastop and f.function are magic */
static
DFHANDLER(deltastop)
{
	fprintf(stderr, "%s(): Shouldn't get here.\n", __func__);
	return;
}
static
DFHANDLER(function)
{
	fprintf(stderr, "%s(): Shouldn't get here.\n", __func__);
	return;
}
