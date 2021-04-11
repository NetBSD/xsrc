/*
 * Window decoration routines -- initializtion time
 *
 * These are funcs that are called during ctwm initialization to setup
 * bits based on general X stuff and/or config file bits.
 */


#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include "add_window.h"
#include "functions_defs.h"
#include "image.h"
#include "screen.h"

#include "win_decorations_init.h"


/*
 * Global marker used in config file loading to track "which one we're
 * currently messing with"
 */
static TitleButton *cur_tb = NULL;


/* Internal func[s] */
static void ComputeCommonTitleOffsets(void);



/*
 * InitTitlebarButtons - Do all the necessary stuff to load in a titlebar
 * button.  If we can't find the button, then put in a question; if we can't
 * find the question mark, something is wrong and we are probably going to be
 * in trouble later on.
 */
void
InitTitlebarButtons(void)
{
	TitleButton *tb;
	int h;

	/*
	 * initialize dimensions
	 */
	Scr->TBInfo.width = (Scr->TitleHeight -
	                     2 * (Scr->FramePadding + Scr->ButtonIndent));
	if(Scr->use3Dtitles)
		Scr->TBInfo.pad = ((Scr->TitlePadding > 1)
		                   ? ((Scr->TitlePadding + 1) / 2) : 0);
	else
		Scr->TBInfo.pad = ((Scr->TitlePadding > 1)
		                   ? ((Scr->TitlePadding + 1) / 2) : 1);
	h = Scr->TBInfo.width - 2 * Scr->TBInfo.border;

	/*
	 * add in some useful buttons and bindings so that novices can still
	 * use the system.
	 */
	if(!Scr->NoDefaults) {
		/* insert extra buttons */
#define MKBTN(bmap, func, isrt) \
                        CreateTitleButton(TBPM_##bmap, F_##func, "", NULL, \
                                          isrt, isrt)

		/* Iconify on the left, resize on the right */
		if(Scr->use3Dtitles) {
			MKBTN(3DDOT, ICONIFY, false);
			MKBTN(3DRESIZE, RESIZE, true);
		}
		else {
			MKBTN(ICONIFY, ICONIFY, false);
			MKBTN(RESIZE, RESIZE, true);
		}

#undef MKBTN

		/* Default mouse bindings in titlebar/icon/iconmgr as fallback */
		AddDefaultFuncButtons();
	}

	/* Init screen-wide dimensions for common titlebar bits */
	ComputeCommonTitleOffsets();


	/*
	 * load in images and do appropriate centering
	 */
	for(tb = Scr->TBInfo.head; tb; tb = tb->next) {
		tb->image = GetImage(tb->name, Scr->TitleC);
		if(!tb->image) {
			/* Couldn't find it, make a question mark */
			tb->image = GetImage(TBPM_QUESTION, Scr->TitleC);
			if(!tb->image) {
				/*
				 * (sorta) Can't Happen.  Calls a static function that
				 * builds from static data, so could only possibly fail
				 * if XCreateBitmapFromData() failed (which should be
				 * vanishingly rare; memory allocation failures etc).
				 */
				fprintf(stderr, "%s:  unable to add titlebar button \"%s\"\n",
				        ProgramName, tb->name);
				continue;
			}
		}
		tb->width  = tb->image->width;
		tb->height = tb->image->height;

		/* Figure centering.  Horizontally... */
		tb->dstx = (h - tb->width + 1) / 2;
		if(tb->dstx < 0) {              /* clip to minimize copying */
			tb->srcx = -(tb->dstx);
			tb->width = h;
			tb->dstx = 0;
		}
		else {
			tb->srcx = 0;
		}

		/* ... and vertically */
		tb->dsty = (h - tb->height + 1) / 2;
		if(tb->dsty < 0) {
			tb->srcy = -(tb->dsty);
			tb->height = h;
			tb->dsty = 0;
		}
		else {
			tb->srcy = 0;
		}
	}
}


/*
 * Figure general sizing/locations for titlebar bits.
 *
 * For the session; called during ctwm startup.  main() ->
 * InitTitleBarButtons() -> ComputeCommonTitleOffsets()
 */
static void
ComputeCommonTitleOffsets(void)
{
	int buttonwidth = (Scr->TBInfo.width + Scr->TBInfo.pad);

	/* Start "+left" and "-right" with our padding */
	Scr->TBInfo.leftx = Scr->TBInfo.rightoff = Scr->FramePadding;

	/*
	 * If there are buttons on the left, add a space to clear the right
	 * edge of the last one.
	 */
	if(Scr->TBInfo.nleft > 0) {
		Scr->TBInfo.leftx += Scr->ButtonIndent;
	}

	/*
	 * Similar on the right, except we need to know how many there are
	 * and account for all of that to leave enough space open for them.
	 * We didn't need to do that above because leftx is already relative
	 * to the end of the window holding them (and so means something like
	 * "move over this much further"), whereas rightoff is relative to
	 * the right side of the titlebar (and so means something like "we
	 * have to leave this much space")?
	 */
	if(Scr->TBInfo.nright > 0) {
		Scr->TBInfo.rightoff += (Scr->ButtonIndent
		                         + (Scr->TBInfo.nright * buttonwidth)
		                         - Scr->TBInfo.pad);
	}

	/*
	 * titlex does however go from the far-left of the titlebar, so it
	 * needs to account for the space the left-side buttons use.
	 */
	Scr->TBInfo.titlex = (Scr->TBInfo.leftx
	                      + (Scr->TBInfo.nleft * buttonwidth)
	                      - Scr->TBInfo.pad
	                      + Scr->TitlePadding);
}



/*
 * Sets the action for a given {mouse button,set of modifier keys} on the
 * "current" button.  This happens during initialization, in a few
 * different ways.
 *
 * CreateTitleButton() winds up creating a new button, and setting the
 * cur_tb global we rely on.  It calls us then to initialize our action
 * to what it was told (hardcoded for the !NoDefaults case in
 * InitTitlebarButtons() for fallback config, from the config file when
 * it's called via GotTitleButton() for the one-line string form of
 * *TitleButton spec).
 *
 * It's also called directly from the config parsing for the block-form
 * *TitleButton specs, when the cur_tb was previously set by
 * CreateTitleButton() at the opening of the block.
 */
void
SetCurrentTBAction(int button, int nmods, int func, char *action,
                   MenuRoot *menuroot)
{
	TitleButtonFunc *tbf;

	if(!cur_tb) {
		fprintf(stderr, "%s: can't find titlebutton\n", ProgramName);
		return;
	}
	for(tbf = cur_tb->funs; tbf; tbf = tbf->next) {
		if(tbf->num == button && tbf->mods == nmods) {
			break;
		}
	}
	if(!tbf) {
		tbf = malloc(sizeof(TitleButtonFunc));
		if(!tbf) {
			fprintf(stderr, "%s: out of memory\n", ProgramName);
			return;
		}
		tbf->next = cur_tb->funs;
		cur_tb->funs = tbf;
	}
	tbf->num = button;
	tbf->mods = nmods;
	tbf->func = func;
	tbf->action = action;
	tbf->menuroot = menuroot;
}



/*
 * XXX This return value is a little pointless.  The only failure it
 * acknowledges is from malloc(), and that Never Fails On Real
 * Systems(tm).  And if it does, we're pretty screwed anyway.
 */
bool
CreateTitleButton(char *name, int func, char *action, MenuRoot *menuroot,
                  bool rightside, bool append)
{
	int button;
	cur_tb = calloc(1, sizeof(TitleButton));

	if(!cur_tb) {
		fprintf(stderr,
		        "%s:  unable to allocate %lu bytes for title button\n",
		        ProgramName, (unsigned long) sizeof(TitleButton));
		return false;
	}

	cur_tb->name = name;           /* note that we are not copying */
	cur_tb->rightside = rightside;
	if(rightside) {
		Scr->TBInfo.nright++;
	}
	else {
		Scr->TBInfo.nleft++;
	}

	for(button = 0; button < MAX_BUTTONS; button++) {
		SetCurrentTBAction(button + 1, 0, func, action, menuroot);
	}

	/*
	 * Cases for list:
	 *
	 *     1.  empty list, prepend left       put at head of list
	 *     2.  append left, prepend right     put in between left and right
	 *     3.  append right                   put at tail of list
	 *
	 * Do not refer to widths and heights yet since buttons not created
	 * (since fonts not loaded and heights not known).
	 */
	if((!Scr->TBInfo.head) || ((!append) && (!rightside))) {    /* 1 */
		cur_tb->next = Scr->TBInfo.head;
		Scr->TBInfo.head = cur_tb;
	}
	else if(append && rightside) {      /* 3 */
		TitleButton *t;
		for(t = Scr->TBInfo.head; t->next; t = t->next) {
			/* just walking to tail */;
		}
		t->next = cur_tb;
		cur_tb->next = NULL;
	}
	else {                              /* 2 */
		TitleButton *t, *prev = NULL;
		for(t = Scr->TBInfo.head; t && !t->rightside; t = t->next) {
			prev = t;
		}
		if(prev) {
			cur_tb->next = prev->next;
			prev->next = cur_tb;
		}
		else {
			cur_tb->next = Scr->TBInfo.head;
			Scr->TBInfo.head = cur_tb;
		}
	}

	return true;
}
