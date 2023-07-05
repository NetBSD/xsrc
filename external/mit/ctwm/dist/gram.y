/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: gram.y,v 1.91 91/02/08 18:21:56 dave Exp $
 *
 * .twmrc command grammer
 *
 * 07-Jan-86 Thomas E. LaStrange	File created
 * 11-Nov-90 Dave Sternlicht            Adding SaveColors
 * 10-Oct-90 David M. Sternlicht        Storing saved colors on root
 * 22-April-92 Claude Lecommandeur	modifications for ctwm.
 *
 ***********************************************************************/

%{
#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "otp.h"
#include "iconmgr.h"
#include "icons.h"
#ifdef WINBOX
#include "windowbox.h"
#endif
#include "functions_defs.h"
#include "list.h"
#include "util.h"
#include "occupation.h"
#include "screen.h"
#include "parse.h"
#include "parse_be.h"
#include "parse_yacc.h"
#include "cursor.h"
#include "win_decorations_init.h"
#include "win_regions.h"
#include "workspace_config.h"
#ifdef SOUNDS
#	include "sound.h"
#endif

static char *curWorkSpc = NULL;
static char *client = NULL;
static char *workspace = NULL;
static MenuItem *lastmenuitem = NULL;
static name_list **curplist = NULL;
static int color = 0;
extern char *yytext; // Have to manually pull this in

int yylex(void);
%}

%union
{
    int num;
    char *ptr;
};


%token <num> LB RB LP RP MENUS MENU BUTTON DEFAULT_FUNCTION PLUS MINUS
%token <num> ALL OR CURSORS PIXMAPS ICONS COLOR SAVECOLOR MONOCHROME FUNCTION
%token <num> ICONMGR_SHOW ICONMGR ALTER WINDOW_FUNCTION ZOOM ICONMGRS
%token <num> ICONMGR_GEOMETRY ICONMGR_NOSHOW MAKE_TITLE
%token <num> ICONIFY_BY_UNMAPPING DONT_ICONIFY_BY_UNMAPPING
%token <num> AUTO_POPUP
%token <num> NO_BORDER NO_ICON_TITLE NO_TITLE AUTO_RAISE NO_HILITE ICON_REGION
%token <num> WINDOW_REGION META SHIFT LOCK CONTROL WINDOW TITLE ICON ROOT FRAME
%token <num> COLON EQUALS SQUEEZE_TITLE DONT_SQUEEZE_TITLE
%token <num> WARP_ON_DEICONIFY
%token <num> START_ICONIFIED NO_TITLE_HILITE TITLE_HILITE
%token <num> MOVE RESIZE WAITC SELECT KILL LEFT_TITLEBUTTON RIGHT_TITLEBUTTON
%token <num> NUMBER KEYWORD NKEYWORD CKEYWORD CLKEYWORD FKEYWORD FSKEYWORD
%token <num> FNKEYWORD PRIORITY_SWITCHING PRIORITY_NOT_SWITCHING
%token <num> SKEYWORD SSKEYWORD WINDOW_RING WINDOW_RING_EXCLUDE WARP_CURSOR ERRORTOKEN
%token <num> GRAVITY /* N/S/E/W */
%token <num> SIJENUM /* SqueezeTitle justifications, SIJust enum */
%token <num> NO_STACKMODE ALWAYS_ON_TOP WORKSPACE WORKSPACES WORKSPCMGR_GEOMETRY
%token <num> OCCUPYALL OCCUPYLIST MAPWINDOWCURRENTWORKSPACE MAPWINDOWDEFAULTWORKSPACE
%token <num> ON_TOP_PRIORITY
%token <num> UNMAPBYMOVINGFARAWAY OPAQUEMOVE NOOPAQUEMOVE OPAQUERESIZE NOOPAQUERESIZE
%token <num> DONTSETINACTIVE CHANGE_WORKSPACE_FUNCTION DEICONIFY_FUNCTION ICONIFY_FUNCTION
%token <num> AUTOSQUEEZE STARTSQUEEZED DONT_SAVE AUTO_LOWER ICONMENU_DONTSHOW WINDOW_BOX
%token <num> IGNOREMODIFIER WINDOW_GEOMETRIES ALWAYSSQUEEZETOGRAVITY VIRTUAL_SCREENS
%token <num> IGNORE_TRANSIENT
%token <num> EWMH_IGNORE
%token <num> MWM_IGNORE
%token <num> MONITOR_LAYOUT
%token <num> RPLAY_SOUNDS
%token <num> FORCE_FOCUS
%token <ptr> STRING

%type <ptr> string
%type <num> action button number signed_number keyaction full fullkey
%type <num> vgrav hgrav

%start twmrc

%%
twmrc		: { InitGramVariables(); }
                  stmts
		;

stmts		: /* Empty */
		| stmts stmt
		;

stmt		: error
		| noarg
		| sarg
		| narg
		| squeeze
		| ICON_REGION string vgrav hgrav number number {
		      AddIconRegion($2, $3, $4, $5, $6, "undef", "undef", "undef");
		  }
		| ICON_REGION string vgrav hgrav number number string {
		      AddIconRegion($2, $3, $4, $5, $6, $7, "undef", "undef");
		  }
		| ICON_REGION string vgrav hgrav number number string string {
		      AddIconRegion($2, $3, $4, $5, $6, $7, $8, "undef");
		  }
		| ICON_REGION string vgrav hgrav number number string string string {
		      AddIconRegion($2, $3, $4, $5, $6, $7, $8, $9);
		  }
		| ICON_REGION string vgrav hgrav number number {
		      curplist = AddIconRegion($2, $3, $4, $5, $6, "undef", "undef", "undef");
		  }
		  win_list
		| ICON_REGION string vgrav hgrav number number string {
		      curplist = AddIconRegion($2, $3, $4, $5, $6, $7, "undef", "undef");
		  }
		  win_list
		| ICON_REGION string vgrav hgrav number number string string {
		      curplist = AddIconRegion($2, $3, $4, $5, $6, $7, $8, "undef");
		  }
		  win_list
		| ICON_REGION string vgrav hgrav number number string string string {
		      curplist = AddIconRegion($2, $3, $4, $5, $6, $7, $8, $9);
		  }
		  win_list

		| WINDOW_REGION string vgrav hgrav {
		      curplist = AddWindowRegion ($2, $3, $4);
		  }
		  win_list

		| WINDOW_BOX string string {
#ifdef WINBOX
		      curplist = addWindowBox ($2, $3);
#endif
		  }
		  win_list

		| ICONMGR_GEOMETRY string number	{ if (Scr->FirstTime)
						  {
						    Scr->iconmgr->geometry= (char*)$2;
						    Scr->iconmgr->columns=$3;
						  }
						}
		| ICONMGR_GEOMETRY string	{ if (Scr->FirstTime)
						    Scr->iconmgr->geometry = (char*)$2;
						}
		| WORKSPCMGR_GEOMETRY string number	{ if (Scr->FirstTime)
				{
				    Scr->workSpaceMgr.geometry= (char*)$2;
				    Scr->workSpaceMgr.columns=$3;
				}
						}
		| WORKSPCMGR_GEOMETRY string	{ if (Scr->FirstTime)
				    Scr->workSpaceMgr.geometry = (char*)$2;
						}
		| MAPWINDOWCURRENTWORKSPACE {}
		  curwork

		| MAPWINDOWDEFAULTWORKSPACE {}
		  defwork

		| ZOOM number		{ if (Scr->FirstTime)
					  {
						Scr->DoZoom = true;
						Scr->ZoomCount = $2;
					  }
					}
		| ZOOM			{ if (Scr->FirstTime)
						Scr->DoZoom = true; }
		| PIXMAPS pixmap_list	{}
		| CURSORS cursor_list	{}
		| ICONIFY_BY_UNMAPPING	{ curplist = &Scr->IconifyByUn; }
		  win_list
		| ICONIFY_BY_UNMAPPING	{ if (Scr->FirstTime)
		    Scr->IconifyByUnmapping = true; }

		| OPAQUEMOVE	{ curplist = &Scr->OpaqueMoveList; }
		  win_list
		| OPAQUEMOVE	{ if (Scr->FirstTime) Scr->DoOpaqueMove = true; }
		| NOOPAQUEMOVE	{ curplist = &Scr->NoOpaqueMoveList; }
		  win_list
		| NOOPAQUEMOVE	{ if (Scr->FirstTime) Scr->DoOpaqueMove = false; }
		| OPAQUERESIZE	{ curplist = &Scr->OpaqueMoveList; }
		  win_list
		| OPAQUERESIZE	{ if (Scr->FirstTime) Scr->DoOpaqueResize = true; }
		| NOOPAQUERESIZE	{ curplist = &Scr->NoOpaqueResizeList; }
		  win_list
		| NOOPAQUERESIZE	{ if (Scr->FirstTime) Scr->DoOpaqueResize = false; }

		| LEFT_TITLEBUTTON string EQUALS action {
					  GotTitleButton ($2, $4, false);
					}
		| RIGHT_TITLEBUTTON string EQUALS action {
					  GotTitleButton ($2, $4, true);
					}
		| LEFT_TITLEBUTTON string { CreateTitleButton($2, 0, NULL, NULL, false, true); }
		  binding_list
		| RIGHT_TITLEBUTTON string { CreateTitleButton($2, 0, NULL, NULL, true, true); }
		  binding_list
		| button string		{
		    root = GetRoot($2, NULL, NULL);
		    AddFuncButton ($1, C_ROOT, 0, F_MENU, root, NULL);
		}
		| button action		{
			if ($2 == F_MENU) {
			    pull->prev = NULL;
			    AddFuncButton ($1, C_ROOT, 0, $2, pull, NULL);
			}
			else {
			    MenuItem *item;

			    root = GetRoot(TWM_ROOT,NULL,NULL);
			    item = AddToMenu (root, "x", Action,
					NULL, $2, NULL, NULL);
			    AddFuncButton ($1, C_ROOT, 0, $2, NULL, item);
			}
			Action = "";
			pull = NULL;
		}
		| string fullkey	{ GotKey($1, $2); }
		| button full		{ GotButton($1, $2); }

		| DONT_ICONIFY_BY_UNMAPPING { curplist = &Scr->DontIconify; }
		  win_list
		| WORKSPACES {}
		  workspc_list
		| IGNOREMODIFIER
			{ mods = 0; }
			LB keys
			{ Scr->IgnoreModifier |= mods; mods = 0; }
			RB
		| OCCUPYALL		{ curplist = &Scr->OccupyAll; }
		  win_list
		| ICONMENU_DONTSHOW	{ curplist = &Scr->IconMenuDontShow; }
		  win_list
		| OCCUPYLIST {}
		  occupy_list
		| UNMAPBYMOVINGFARAWAY	{ curplist = &Scr->UnmapByMovingFarAway; }
		  win_list
		| AUTOSQUEEZE		{ curplist = &Scr->AutoSqueeze; }
		  win_list
		| STARTSQUEEZED		{ curplist = &Scr->StartSqueezed; }
		  win_list
		| ALWAYSSQUEEZETOGRAVITY	{ Scr->AlwaysSqueezeToGravity = true; }
		| ALWAYSSQUEEZETOGRAVITY	{ curplist = &Scr->AlwaysSqueezeToGravityL; }
		  win_list
		| DONTSETINACTIVE	{ curplist = &Scr->DontSetInactive; }
		  win_list
		| ICONMGR_NOSHOW	{ curplist = &Scr->IconMgrNoShow; }
		  win_list
		| ICONMGR_NOSHOW	{ Scr->IconManagerDontShow = true; }
		| ICONMGRS		{ curplist = &Scr->IconMgrs; }
		  iconm_list
		| ICONMGR_SHOW		{ curplist = &Scr->IconMgrShow; }
		  win_list
		| NO_TITLE_HILITE	{ curplist = &Scr->NoTitleHighlight; }
		  win_list
		| NO_TITLE_HILITE	{ if (Scr->FirstTime)
						Scr->TitleHighlight = false; }
		| NO_HILITE		{ curplist = &Scr->NoHighlight; }
		  win_list
		| NO_HILITE		{ if (Scr->FirstTime)
						Scr->Highlight = false; }
                | ON_TOP_PRIORITY signed_number 
                                        { OtpScrSetZero(Scr, WinWin, $2); }
		| ON_TOP_PRIORITY ICONS signed_number
                                        { OtpScrSetZero(Scr, IconWin, $3); }
		| ON_TOP_PRIORITY signed_number
                                        { curplist = OtpScrPriorityL(Scr, WinWin, $2); }
		  win_list
		| ON_TOP_PRIORITY ICONS signed_number
                                        { curplist = OtpScrPriorityL(Scr, IconWin, $3); }
		  win_list
		| ALWAYS_ON_TOP		{ curplist = OtpScrPriorityL(Scr, WinWin, 8); }
		  win_list
		| PRIORITY_SWITCHING	{ OtpScrSetSwitching(Scr, WinWin, false);
		                          curplist = OtpScrSwitchingL(Scr, WinWin); }
		  win_list
		| PRIORITY_NOT_SWITCHING { OtpScrSetSwitching(Scr, WinWin, true);
		                          curplist = OtpScrSwitchingL(Scr, WinWin); }
		  win_list
		| PRIORITY_SWITCHING ICONS
                                        { OtpScrSetSwitching(Scr, IconWin, false);
                                        curplist = OtpScrSwitchingL(Scr, IconWin); }
		  win_list
		| PRIORITY_NOT_SWITCHING ICONS
                                        { OtpScrSetSwitching(Scr, IconWin, true);
		                          curplist = OtpScrSwitchingL(Scr, IconWin); }
		  win_list

		  win_list
		| NO_STACKMODE		{ curplist = &Scr->NoStackModeL; }
		  win_list
		| NO_STACKMODE		{ if (Scr->FirstTime)
						Scr->StackMode = false; }
		| NO_BORDER		{ curplist = &Scr->NoBorder; }
		  win_list
		| AUTO_POPUP		{ Scr->AutoPopup = true; }
		| AUTO_POPUP		{ curplist = &Scr->AutoPopupL; }
		  win_list
		| DONT_SAVE		{
#ifndef SESSION
			twmrc_error_prefix();
			fprintf(stderr, "DontSave ignored; session support "
					"disabled.\n");
#endif
				curplist = &Scr->DontSave;
			}
		  win_list
		| NO_ICON_TITLE		{ curplist = &Scr->NoIconTitle; }
		  win_list
		| NO_ICON_TITLE		{ if (Scr->FirstTime)
						Scr->NoIconTitlebar = true; }
		| NO_TITLE		{ curplist = &Scr->NoTitle; }
		  win_list
		| NO_TITLE		{ if (Scr->FirstTime)
						Scr->NoTitlebar = true; }
		| IGNORE_TRANSIENT	{ curplist = &Scr->IgnoreTransientL; }
		  win_list
		| MAKE_TITLE		{ curplist = &Scr->MakeTitle; }
		  win_list
		| START_ICONIFIED	{ curplist = &Scr->StartIconified; }
		  win_list
		| AUTO_RAISE		{ curplist = &Scr->AutoRaise; }
		  win_list
		| AUTO_RAISE		{ Scr->AutoRaiseDefault = true; }
		| WARP_ON_DEICONIFY	{ curplist = &Scr->WarpOnDeIconify; }
		  win_list
		| AUTO_LOWER		{ curplist = &Scr->AutoLower; }
		  win_list
		| AUTO_LOWER		{ Scr->AutoLowerDefault = true; }
		| MENU string LP string COLON string RP	{
					root = GetRoot($2, $4, $6); }
		  menu			{ root->real_menu = true;}
		| MENU string		{ root = GetRoot($2, NULL, NULL); }
		  menu			{ root->real_menu = true; }
		| FUNCTION string	{ root = GetRoot($2, NULL, NULL); }
		  function
		| ICONS			{ curplist = &Scr->IconNames; }
		  icon_list
		| COLOR			{ color = COLOR; }
		  color_list
		| SAVECOLOR		{}
		  save_color_list
		| MONOCHROME		{ color = MONOCHROME; }
		  color_list
		| DEFAULT_FUNCTION action { Scr->DefaultFunction.func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Scr->DefaultFunction.menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT,NULL,NULL);
					    Scr->DefaultFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,$2, NULL, NULL);
					  }
					  Action = "";
					  pull = NULL;
					}
		| WINDOW_FUNCTION action { Scr->WindowFunction.func = $2;
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->WindowFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,$2, NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
		| CHANGE_WORKSPACE_FUNCTION action { Scr->ChangeWorkspaceFunction.func = $2;
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->ChangeWorkspaceFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,$2, NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
		| DEICONIFY_FUNCTION action { Scr->DeIconifyFunction.func = $2;
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->DeIconifyFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,$2, NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
		| ICONIFY_FUNCTION action { Scr->IconifyFunction.func = $2;
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->IconifyFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,$2, NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
		| WARP_CURSOR		{ curplist = &Scr->WarpCursorL; }
		  win_list
		| WARP_CURSOR		{ if (Scr->FirstTime)
					    Scr->WarpCursor = true; }
		| WINDOW_RING		{ curplist = &Scr->WindowRingL; }
		  win_list
		| WINDOW_RING		{ Scr->WindowRingAll = true; }
		| WINDOW_RING_EXCLUDE	{ if (!Scr->WindowRingL)
					    Scr->WindowRingAll = true;
					  curplist = &Scr->WindowRingExcludeL; }
		  win_list
		| WINDOW_GEOMETRIES	{  }
		  wingeom_list
		| VIRTUAL_SCREENS	{ }
		  vscreen_geom_list
		| EWMH_IGNORE		{ }
		  ewmh_ignore_list
		| MWM_IGNORE		{ }
		  mwm_ignore_list
		| MONITOR_LAYOUT { init_layout_override(); }
			layout_geom_list
		| RPLAY_SOUNDS { }
		  rplay_sounds_list
		| FORCE_FOCUS { Scr->ForceFocus = true; }
		| FORCE_FOCUS { curplist = &Scr->ForceFocusL; }
		  win_list
		;

noarg		: KEYWORD		{ if (!do_single_keyword ($1)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
					"unknown singleton keyword %d\n",
						     $1);
					    ParseError = true;
					  }
					}
		;

sarg		: SKEYWORD string	{ if (!do_string_keyword ($1, $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     $1, $2);
					    ParseError = true;
					  }
					}
		| SKEYWORD		{ if (!do_string_keyword ($1, DEFSTRING)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (no value)\n",
						     $1);
					    ParseError = true;
					  }
					}
		;

sarg		: SSKEYWORD string string
					{ if (!do_string_string_keyword ($1, $2, $3)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown strings keyword %d (value \"%s\" and \"%s\")\n",
						     $1, $2, $3);
					    ParseError = true;
					  }
					}
		| SSKEYWORD string	{ if (!do_string_string_keyword ($1, $2, NULL)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     $1, $2);
					    ParseError = true;
					  }
					}
		| SSKEYWORD		{ if (!do_string_string_keyword ($1, NULL, NULL)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (no value)\n",
						     $1);
					    ParseError = true;
					  }
					}
		;

narg		: NKEYWORD number	{ if (!do_number_keyword ($1, $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown numeric keyword %d (value %d)\n",
						     $1, $2);
					    ParseError = true;
					  }
					}
		;



keyaction	: EQUALS keys COLON action  { $$ = $4; }
		;

full		: EQUALS keys COLON contexts COLON action  { $$ = $6; }
		;

fullkey		: EQUALS keys COLON contextkeys COLON action  { $$ = $6; }
		;

keys		: /* Empty */
		| keys key
		;

key		: META			{ mods |= Mod1Mask; }
		| SHIFT			{ mods |= ShiftMask; }
		| LOCK			{ mods |= LockMask; }
		| CONTROL		{ mods |= ControlMask; }
		| ALTER number		{ if ($2 < 1 || $2 > 5) {
					     twmrc_error_prefix();
					     fprintf (stderr,
				"bad altkeymap number (%d), must be 1-5\n",
						      $2);
					     ParseError = true;
					  } else {
					     mods |= (Alt1Mask << ($2 - 1));
					  }
					}
		| META number		{ if ($2 < 1 || $2 > 5) {
					     twmrc_error_prefix();
					     fprintf (stderr,
				"bad modifier number (%d), must be 1-5\n",
						      $2);
					     ParseError = true;
					  } else {
					     mods |= (Mod1Mask << ($2 - 1));
					  }
					}
		| OR			{ }
		;

vgrav	: GRAVITY {
			switch($1) {
				case GRAV_NORTH:
				case GRAV_SOUTH:
					/* OK */
					$$ = $1;
					break;
				default:
					twmrc_error_prefix();
					fprintf(stderr, "Bad vertical gravity '%s'\n", yytext);
					ParseError = true;
					YYERROR;
			}
		}

hgrav	: GRAVITY {
			switch($1) {
				case GRAV_EAST:
				case GRAV_WEST:
					/* OK */
					$$ = $1;
					break;
				default:
					twmrc_error_prefix();
					fprintf(stderr, "Bad horiz gravity '%s'\n", yytext);
					ParseError = true;
					YYERROR;
			}
		}

contexts	: /* Empty */
		| contexts context
		;

context		: WINDOW		{ cont |= C_WINDOW_BIT; }
		| TITLE			{ cont |= C_TITLE_BIT; }
		| ICON			{ cont |= C_ICON_BIT; }
		| ROOT			{ cont |= C_ROOT_BIT; }
		| FRAME			{ cont |= C_FRAME_BIT; }
		| WORKSPACE		{ cont |= C_WORKSPACE_BIT; }
		| ICONMGR		{ cont |= C_ICONMGR_BIT; }
		| META			{ cont |= C_ICONMGR_BIT; }
		| ALTER			{ cont |= C_ALTER_BIT; }
		| ALL			{ cont |= C_ALL_BITS; }
		| OR			{ }
		;

contextkeys	: /* Empty */
		| contextkeys contextkey
		;

contextkey	: WINDOW		{ cont |= C_WINDOW_BIT; }
		| TITLE			{ cont |= C_TITLE_BIT; }
		| ICON			{ cont |= C_ICON_BIT; }
		| ROOT			{ cont |= C_ROOT_BIT; }
		| FRAME			{ cont |= C_FRAME_BIT; }
		| WORKSPACE		{ cont |= C_WORKSPACE_BIT; }
		| ICONMGR		{ cont |= C_ICONMGR_BIT; }
		| META			{ cont |= C_ICONMGR_BIT; }
		| ALTER			{ cont |= C_ALTER_BIT; }
		| ALL			{ cont |= C_ALL_BITS; }
		| OR			{ }
		| string		{ Name = (char*)$1; cont |= C_NAME_BIT; }
		;


binding_list    : LB binding_entries RB {}
		;

binding_entries : /* Empty */
		| binding_entries binding_entry
		;

binding_entry   : button keyaction { SetCurrentTBAction($1, mods, $2, Action, pull); mods = 0;}
		| button EQUALS action { SetCurrentTBAction($1, 0, $3, Action, pull);}
		| button COLON action {
			/* Deprecated since 3.8, no longer supported */
			yyerror("Title buttons specifications without = are no "
			        "longer supported.");
		}
		;


pixmap_list	: LB pixmap_entries RB {}
		;

pixmap_entries	: /* Empty */
		| pixmap_entries pixmap_entry
		;

pixmap_entry	: TITLE_HILITE string { Scr->HighlightPixmapName = strdup($2); }
		;


cursor_list	: LB cursor_entries RB {}
		;

cursor_entries	: /* Empty */
		| cursor_entries cursor_entry
		;

cursor_entry	: FRAME string string {
			NewBitmapCursor(&Scr->FrameCursor, $2, $3); }
		| FRAME string	{
			NewFontCursor(&Scr->FrameCursor, $2); }
		| TITLE string string {
			NewBitmapCursor(&Scr->TitleCursor, $2, $3); }
		| TITLE string {
			NewFontCursor(&Scr->TitleCursor, $2); }
		| ICON string string {
			NewBitmapCursor(&Scr->IconCursor, $2, $3); }
		| ICON string {
			NewFontCursor(&Scr->IconCursor, $2); }
		| ICONMGR string string {
			NewBitmapCursor(&Scr->IconMgrCursor, $2, $3); }
		| ICONMGR string {
			NewFontCursor(&Scr->IconMgrCursor, $2); }
		| BUTTON string string {
			NewBitmapCursor(&Scr->ButtonCursor, $2, $3); }
		| BUTTON string {
			NewFontCursor(&Scr->ButtonCursor, $2); }
		| MOVE string string {
			NewBitmapCursor(&Scr->MoveCursor, $2, $3); }
		| MOVE string {
			NewFontCursor(&Scr->MoveCursor, $2); }
		| RESIZE string string {
			NewBitmapCursor(&Scr->ResizeCursor, $2, $3); }
		| RESIZE string {
			NewFontCursor(&Scr->ResizeCursor, $2); }
		| WAITC string string {
			NewBitmapCursor(&Scr->WaitCursor, $2, $3); }
		| WAITC string {
			NewFontCursor(&Scr->WaitCursor, $2); }
		| MENU string string {
			NewBitmapCursor(&Scr->MenuCursor, $2, $3); }
		| MENU string {
			NewFontCursor(&Scr->MenuCursor, $2); }
		| SELECT string string {
			NewBitmapCursor(&Scr->SelectCursor, $2, $3); }
		| SELECT string {
			NewFontCursor(&Scr->SelectCursor, $2); }
		| KILL string string {
			NewBitmapCursor(&Scr->DestroyCursor, $2, $3); }
		| KILL string {
			NewFontCursor(&Scr->DestroyCursor, $2); }
		;

color_list	: LB color_entries RB {}
		;


color_entries	: /* Empty */
		| color_entries color_entry
		;

color_entry	: CLKEYWORD string	{ if (!do_colorlist_keyword ($1, color,
								     $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled list color keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = true;
					  }
					}
		| CLKEYWORD string	{ curplist = do_colorlist_keyword($1,color,
								      $2);
					  if (!curplist) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color list keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = true;
					  }
					}
		  win_color_list
		| CKEYWORD string	{ if (!do_color_keyword ($1, color,
								 $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = true;
					  }
					}
		;

save_color_list : LB s_color_entries RB {}
		;

s_color_entries : /* Empty */
		| s_color_entries s_color_entry
		;

s_color_entry   : string		{ do_string_savecolor(color, $1); }
		| CLKEYWORD		{ do_var_savecolor($1); }
		;

win_color_list	: LB win_color_entries RB {}
		;

win_color_entries	: /* Empty */
		| win_color_entries win_color_entry
		;

win_color_entry	: string string		{ if (Scr->FirstTime &&
					      color == Scr->Monochrome)
					    AddToList(curplist, $1, $2); }
		;

wingeom_list	: LB wingeom_entries RB {}
		;

wingeom_entries	: /* Empty */
		| wingeom_entries wingeom_entry
		;
/* added a ';' after call to AddToList */
wingeom_entry	: string string	{ AddToList (&Scr->WindowGeometries, $1, $2); }
		;

vscreen_geom_list	: LB vscreen_geom_entries RB {}
		;

vscreen_geom_entries	: /* Empty */
		| vscreen_geom_entries vscreen_geom_entry
		;

vscreen_geom_entry	: string {
#ifdef VSCREEN
				   AddToList (&Scr->VirtualScreens, $1, "");
#endif
				   }
		;


ewmh_ignore_list	: LB ewmh_ignore_entries RB { proc_ewmh_ignore(); }
		;

ewmh_ignore_entries	: /* Empty */
		| ewmh_ignore_entries ewmh_ignore_entry
		;

ewmh_ignore_entry	: string { add_ewmh_ignore($1); }
		;


mwm_ignore_list	: LB mwm_ignore_entries RB { proc_mwm_ignore(); }
		;

mwm_ignore_entries	: /* Empty */
		| mwm_ignore_entries mwm_ignore_entry
		;

mwm_ignore_entry	: string { add_mwm_ignore($1); }
		;


layout_geom_list	: LB layout_geom_entries RB { proc_layout_override(); }
		;

layout_geom_entries	: /* Empty */
		| layout_geom_entries layout_geom_entry
		;

layout_geom_entry	: string { add_layout_override_entry($1); }
		;


squeeze		: SQUEEZE_TITLE {
				    if (HasShape) Scr->SqueezeTitle = true;
				}
		| SQUEEZE_TITLE { curplist = &Scr->SqueezeTitleL;
				  if (HasShape)
				    Scr->SqueezeTitle = true;
				}
		  LB win_sqz_entries RB
		| DONT_SQUEEZE_TITLE { Scr->SqueezeTitle = false; }
		| DONT_SQUEEZE_TITLE { curplist = &Scr->DontSqueezeTitleL; }
		  win_list
		;

win_sqz_entries	: /* Empty */
		| win_sqz_entries string SIJENUM signed_number number	{
				if (Scr->FirstTime) {
				   do_squeeze_entry (curplist, $2, $3, $4, $5);
				}
			}
		;


iconm_list	: LB iconm_entries RB {}
		;

iconm_entries	: /* Empty */
		| iconm_entries iconm_entry
		;

iconm_entry	: string string number	{ if (Scr->FirstTime)
					    AddToList(curplist, $1,
						AllocateIconManager($1, NULL,
							$2,$3));
					}
		| string string string number
					{ if (Scr->FirstTime)
					    AddToList(curplist, $1,
						AllocateIconManager($1,$2,
						$3, $4));
					}
		;

workspc_list	: LB workspc_entries RB {}
		;

workspc_entries	: /* Empty */
		| workspc_entries workspc_entry
		;

workspc_entry	: string	{
			AddWorkSpace ($1, NULL, NULL, NULL, NULL, NULL);
		}
		| string	{
			curWorkSpc = (char*)$1;
		}
		workapp_list
		;

workapp_list	: LB workapp_entries RB {}
		;

workapp_entries	: /* Empty */
		| workapp_entries workapp_entry
		;

workapp_entry	: string		{
			AddWorkSpace (curWorkSpc, $1, NULL, NULL, NULL, NULL);
		}
		| string string		{
			AddWorkSpace (curWorkSpc, $1, $2, NULL, NULL, NULL);
		}
		| string string string	{
			AddWorkSpace (curWorkSpc, $1, $2, $3, NULL, NULL);
		}
		| string string string string	{
			AddWorkSpace (curWorkSpc, $1, $2, $3, $4, NULL);
		}
		| string string string string string	{
			AddWorkSpace (curWorkSpc, $1, $2, $3, $4, $5);
		}
		;

curwork		: LB string RB {
		    WMapCreateCurrentBackGround ($2, NULL, NULL, NULL);
		}
		| LB string string RB {
		    WMapCreateCurrentBackGround ($2, $3, NULL, NULL);
		}
		| LB string string string RB {
		    WMapCreateCurrentBackGround ($2, $3, $4, NULL);
		}
		| LB string string string string RB {
		    WMapCreateCurrentBackGround ($2, $3, $4, $5);
		}
		;

defwork		: LB string RB {
		    WMapCreateDefaultBackGround ($2, NULL, NULL, NULL);
		}
		| LB string string RB {
		    WMapCreateDefaultBackGround ($2, $3, NULL, NULL);
		}
		| LB string string string RB {
		    WMapCreateDefaultBackGround ($2, $3, $4, NULL);
		}
		| LB string string string string RB {
		    WMapCreateDefaultBackGround ($2, $3, $4, $5);
		}
		;

win_list	: LB win_entries RB {}
		;

win_entries	: /* Empty */
		| win_entries win_entry
		;

win_entry	: string		{ if (Scr->FirstTime)
					    AddToList(curplist, $1, 0);
					}
		;

occupy_list	: LB occupy_entries RB {}
		;

occupy_entries	:  /* Empty */
		| occupy_entries occupy_entry
		;

occupy_entry	: string {client = (char*)$1;}
		  occupy_workspc_list
		| WINDOW    string {client = (char*)$2;}
		  occupy_workspc_list
		| WORKSPACE string {workspace = (char*)$2;}
		  occupy_window_list
		;

occupy_workspc_list	: LB occupy_workspc_entries RB {}
			;

occupy_workspc_entries	:   /* Empty */
			| occupy_workspc_entries occupy_workspc_entry
			;

occupy_workspc_entry	: string {
				if(!AddToClientsList ($1, client)) {
					twmrc_error_prefix();
					fprintf(stderr, "unknown workspace '%s'\n", $1);
				}
			  }
			;

occupy_window_list	: LB occupy_window_entries RB {}
			;

occupy_window_entries	:   /* Empty */
			| occupy_window_entries occupy_window_entry
			;

occupy_window_entry	: string {
				if(!AddToClientsList (workspace, $1)) {
					twmrc_error_prefix();
					fprintf(stderr, "unknown workspace '%s'\n", workspace);
				}
			  }
			;

icon_list	: LB icon_entries RB {}
		;

icon_entries	: /* Empty */
		| icon_entries icon_entry
		;

icon_entry	: string string		{ if (Scr->FirstTime) AddToList(curplist, $1, $2); }
		;

rplay_sounds_list	: LB rplay_sounds_entries RB {
#ifndef SOUNDS
			twmrc_error_prefix();
			fprintf(stderr, "RplaySounds ignored; rplay support "
					"not configured.\n");
#else
			sound_set_from_config();
#endif
		}
		;

rplay_sounds_entries	: /* Empty */
		| rplay_sounds_entries rplay_sounds_entry
		;

rplay_sounds_entry	: string string {
#ifdef SOUNDS
			if(set_sound_event_name($1, $2) != 0) {
				twmrc_error_prefix();
				fprintf(stderr, "Failed adding sound for %s; "
						"maybe event name is invalid?\n", $1);
			}
#endif
		}
		;

function	: LB function_entries RB {}
		;

function_entries: /* Empty */
		| function_entries function_entry
		;

function_entry	: action		{ AddToMenu(root, "", Action, NULL, $1,
						    NULL, NULL);
					  Action = "";
					}
		;

menu		: LB menu_entries RB {lastmenuitem = NULL;}
		;

menu_entries	: /* Empty */
		| menu_entries menu_entry
		;

menu_entry	: string action		{
			if ($2 == F_SEPARATOR) {
			    if (lastmenuitem) lastmenuitem->separated = true;
			}
			else {
			    lastmenuitem = AddToMenu(root, $1, Action, pull, $2, NULL, NULL);
			    Action = "";
			    pull = NULL;
			}
		}
		| string LP string COLON string RP action {
			if ($7 == F_SEPARATOR) {
			    if (lastmenuitem) lastmenuitem->separated = true;
			}
			else {
			    lastmenuitem = AddToMenu(root, $1, Action, pull, $7, $3, $5);
			    Action = "";
			    pull = NULL;
			}
		}
		;

action		: FKEYWORD	{ $$ = $1; }
		| FSKEYWORD string {
				$$ = $1;
				Action = (char*)$2;
				switch ($1) {
				  case F_MENU:
				    pull = GetRoot ($2, NULL,NULL);
				    pull->prev = root;
				    break;
				  case F_WARPRING:
				    if (!CheckWarpRingArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.warptoring argument \"%s\"\n",
						 Action);
					$$ = F_NOP;
				    }
				    break;
				  case F_WARPTOSCREEN:
				    if (!CheckWarpScreenArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.warptoscreen argument \"%s\"\n",
						 Action);
					$$ = F_NOP;
				    }
				    break;
				  case F_COLORMAP:
				    if (CheckColormapArg (Action)) {
					$$ = F_COLORMAP;
				    } else {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.colormap argument \"%s\"\n",
						 Action);
					$$ = F_NOP;
				    }
				    break;
				} /* end switch */
				   }
		;


signed_number	: number		{ $$ = $1; }
		| PLUS number		{ $$ = $2; }
		| MINUS number		{ $$ = -($2); }
		;

button		: BUTTON number		{ $$ = $2;
					  if ($2 == 0)
						yyerror("bad button 0");

					  if ($2 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror("button number too large");
					  }
					}
		;

string		: STRING		{ char *ptr = strdup($1);
					  RemoveDQuote(ptr);
					  $$ = ptr;
					}
		;

number		: NUMBER		{ $$ = $1; }
		;

%%
