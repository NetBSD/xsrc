/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */


/***********************************************************************
 *
 * $XConsortium: parse.c,v 1.52 91/07/12 09:59:37 dave Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange       File created
 * 10-Oct-90 David M. Sternlicht       Storing saved colors on root
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 ***********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#if defined(sony_news) || defined __QNX__
#  include <ctype.h>
#endif
#ifdef VMS
#include <ctype.h>
#include <decw$include/Xos.h>
#include <X11Xmu/CharSet.h>
#include <X11Xmu/SysUtil.h>
#else
#include <X11/Xos.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/SysUtil.h>
#endif
#include "twm.h"
#include "screen.h"
#include "menus.h"
#include "util.h"
#include "parse.h"
#include "version.h"
#ifdef SOUNDS
#  include "sound.h"
#endif
#ifdef VMS
#  include <decw$include/Xatom.h> 
#else
#  include <X11/Xatom.h> 
#endif

/* For m4... */
#ifdef USEM4
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

extern int GoThroughM4;
extern char *keepM4_filename;
extern int KeepTmpFile;
#endif

#if defined(ultrix)
#define NOSTEMP
#endif

#ifndef SYSTEM_INIT_FILE
#ifdef VMS
#define SYSTEM_INIT_FILE "DECW$SYSTEM_DEFAULTS:SYSTEM.CTWMRC"
#else
#define SYSTEM_INIT_FILE "/usr/lib/X11/twm/system.twmrc"
#endif
#endif
#define BUF_LEN 300

static int ParseRandomPlacement (register char *s);
static int ParseButtonStyle (register char *s);
extern int yyparse(void);
extern void twmrc_error_prefix(void);

static FILE *twmrc;
static int ptr = 0;
static int len = 0;
static char buff[BUF_LEN+1];
static char overflowbuff[20];		/* really only need one */
static int overflowlen;
static char **stringListSource, *currentString;
static int ParseUsePPosition (register char *s);
#ifdef USEM4
static FILE *start_m4(FILE *fraw);
static char *m4_defs(Display *display, char *host);
#endif

extern int mods;

int ConstrainedMoveTime = 400;		/* milliseconds, event times */

int RaiseDelay = 0;			/* msec, for AutoRaise */

static int twmStringListInput(void);
#ifndef USEM4
static int twmFileInput(void);
#else
static int m4twmFileInput (void);
#endif
int (*twmInputFunc)(void);

extern char *defTwmrc[];		/* default bindings */

extern char *captivename;

/***********************************************************************
 *
 *  Procedure:
 *	ParseTwmrc - parse the .twmrc file
 *
 *  Inputs:
 *	filename  - the filename to parse.  A NULL indicates $HOME/.twmrc
 *
 ***********************************************************************
 */

#ifdef YYDEBUG
int yydebug = 1;
#endif

static int doparse (int (*ifunc)(void), char *srctypename, char *srcname)
{
    mods = 0;
    ptr = 0;
    len = 0;
    twmrc_lineno = 0;
    ParseError = FALSE;
    twmInputFunc = ifunc;
    overflowlen = 0;

    yyparse();

    if (ParseError) {
	fprintf (stderr, "%s:  errors found in twm %s",
		 ProgramName, srctypename);
	if (srcname) fprintf (stderr, " \"%s\"", srcname);
	fprintf (stderr, "\n");
    }
    return (ParseError ? 0 : 1);
}


int ParseTwmrc (char *filename)
{
    int i;
    char *home = NULL;
    int homelen = 0;
    char *cp = NULL;
    char tmpfilename[257];
#ifdef USEM4
    static FILE *raw;
#endif

    /*
     * Check for the twmrc file in the following order:
     *       Unix                  |   VMS
     *   0.  -f filename.#         | -f filename_#
     *   1.  -f filename           | -f filename
     *   2.  .ctwmrc.#             | ctwm.rc_#
     *   3.  .ctwmrc               | ctwm.rc
     *   4.  .twmrc.#              | twm.rc_#
     *   5.  .twmrc                | twm.rc
     *   6.  system.ctwmrc         | system.ctwmrc
     */
    for (twmrc = NULL, i = 0; !twmrc && i < 7; i++) {
	switch (i) {
#ifdef VMS
	  case 0:
	    if (filename != NULL)  {
	       cp = tmpfilename;
	       (void) sprintf (tmpfilename, "%s_%d", filename, Scr->screen);
	    } else
	       cp = filename;
	    break;

	  case 1:
	    cp = filename;
	    break;

	  case 2:
	    if (!filename) {
		home = getenv ("DECW$USER_DEFAULTS");
		if (home) {
		    homelen = strlen (home);
		    cp = tmpfilename;
		    (void) sprintf (tmpfilename, "%sctwm.rc_%d",
				    home, Scr->screen);
		    break;
		}
	    }
	    continue;

	  case 3:
	    if (home) {
		tmpfilename[homelen + 7] = '\0';
	    }
	    break;

	  case 4:
	    if (!filename) {
		home = getenv ("DECW$USER_DEFAULTS");
		if (home) {
		    homelen = strlen (home);
		    cp = tmpfilename;
		    (void) sprintf (tmpfilename, "%stwm.rc_%d",
				    home, Scr->screen);
		    break;
		}
	    }
	    continue;

	  case 5:
	    if (home) {
		tmpfilename[homelen + 6] = '\0';
	    }
	    break;
#else
	  case 0:			/* -f filename.# */
	    if (filename) {
		cp = tmpfilename;
		(void) sprintf (tmpfilename, "%s.%d", filename, Scr->screen);
	    }
	    else cp = filename;
	    break;

	  case 1:			/* -f filename */
	    cp = filename;
	    break;

	  case 2:			/* ~/.ctwmrc.screennum */
	    if (!filename) {
		home = getenv ("HOME");
		if (home) {
		    homelen = strlen (home);
		    cp = tmpfilename;
		    (void) sprintf (tmpfilename, "%s/.ctwmrc.%d",
				    home, Scr->screen);
		    break;
		}
	    }
	    continue;

	  case 3:			/* ~/.ctwmrc */
	    if (home) {
		tmpfilename[homelen + 8] = '\0';
	    }
	    break;

	  case 4:			/* ~/.twmrc.screennum */
	    if (!filename) {
		home = getenv ("HOME");
		if (home) {
		    homelen = strlen (home);
		    cp = tmpfilename;
		    (void) sprintf (tmpfilename, "%s/.twmrc.%d",
				    home, Scr->screen);
		    break;
		}
	    }
	    continue;

	  case 5:			/* ~/.twmrc */
	    if (home) {
		tmpfilename[homelen + 7] = '\0'; /* C.L. */
	    }
	    break;
#endif

	  case 6:			/* system.twmrc */
	    cp = SYSTEM_INIT_FILE;
	    break;
	}

	if (cp) {
            twmrc = fopen (cp, "r");
#ifdef USEM4
            raw = twmrc;
#endif

        }
    }

#ifdef USEM4
    if (raw) {
#else
    if (twmrc) {
#endif

	int status;

	if (filename && strncmp (cp, filename, strlen (filename))) {
	    fprintf (stderr,
		     "%s:  unable to open twmrc file %s, using %s instead\n",
		     ProgramName, filename, cp);
	}
#ifdef USEM4
	if (GoThroughM4) twmrc = start_m4(raw);
	status = doparse (m4twmFileInput, "file", cp);
	wait (0);
	fclose (twmrc);
        if (GoThroughM4) fclose (raw);
#else
	status = doparse (twmFileInput, "file", cp);
	fclose (twmrc);
#endif
	return status;
    } else {
	if (filename) {
	    fprintf (stderr,
	"%s:  unable to open twmrc file %s, using built-in defaults instead\n",
		     ProgramName, filename);
	}
	return ParseStringList (defTwmrc);
    }
}

int ParseStringList (char **sl)
{
    stringListSource = sl;
    currentString = *sl;
    return doparse (twmStringListInput, "string list", (char *)NULL);
}


/***********************************************************************
 *
 *  Procedure:
 *	twmFileInput - redefinition of the lex input routine for file input
 *
 *  Returned Value:
 *	the next input character
 *
 ***********************************************************************
 */


#ifndef USEM4

/* This has Tom's include() funtionality.  This is utterly useless if you
 * can use m4 for the same thing.               Chris P. Ross */

#define MAX_INCLUDES 10

static struct incl {
     FILE *fp;
     char *name;
     int lineno;
} rc_includes[MAX_INCLUDES];
static int include_file = 0;


static int twmFileInput()
{
    if (overflowlen) return (int) overflowbuff[--overflowlen];

    while (ptr == len)
    {
        while (include_file) {
            if (fgets(buff, BUF_LEN, rc_includes[include_file].fp) == NULL) {
                free(rc_includes[include_file].name);
                fclose(rc_includes[include_file].fp);
                twmrc_lineno = rc_includes[include_file--].lineno;
            } else
                break;
        }

        if (!include_file)
	if (fgets(buff, BUF_LEN, twmrc) == NULL)
	    return 0;
	twmrc_lineno++;

        if (strncmp(buff, "include", 7) == 0) {
             /* Whoops, an include file! */
             char *p = buff + 7, *q;
             FILE *fp;

             while (isspace(*p)) p++;
             for (q = p; *q && !isspace(*q); q++)
                  continue;
             *q = 0;

             if ((fp = fopen(p, "r")) == NULL) {
                  fprintf(stderr, "%s: Unable to open included init file %s\n",
                          ProgramName, p);
                  continue;
             }
             if (++include_file >= MAX_INCLUDES) {
                  fprintf(stderr, "%s: init file includes nested too deep\n",
                          ProgramName);
                  continue;
             }
             rc_includes[include_file].fp = fp;
             rc_includes[include_file].lineno = twmrc_lineno;
             twmrc_lineno = 0;
             rc_includes[include_file].name = malloc(strlen(p)+1);
             strcpy(rc_includes[include_file].name, p);
             continue;
        }
	ptr = 0;
	len = strlen(buff);
    }
    return ((int)buff[ptr++]);
}
#else /* USEM4 */
/* If you're going to use m4, use this version instead.  Much simpler.
 * m4 ism's credit to Josh Osborne (stripes) */

static int m4twmFileInput(void)
{
    int line;
    static FILE *cp = NULL;

    if ( cp == NULL && keepM4_filename ) {
      cp = fopen (keepM4_filename,"w"); 
      if ( cp == NULL ) {
	fprintf (stderr,
		 "%s:  unable to create m4 output %s, ignoring\n",
		 ProgramName, keepM4_filename);
	keepM4_filename = NULL;
      }
    }

    if (overflowlen){
	return((int) overflowbuff[--overflowlen]);
    }

    while (ptr == len) {
	nextline:
	if (fgets(buff, BUF_LEN, twmrc) == NULL) {
	  if ( cp ) fclose (cp);
	  return(0);
	}
	if ( cp ) fputs (buff, cp);

	if (sscanf(buff, "#line %d", &line)) {
	    twmrc_lineno = line - 1;
	    goto nextline;
	} else {
	    twmrc_lineno++;
	}

	ptr = 0;
	len = strlen(buff);
    }
    return ((int)buff[ptr++]);
}
#endif /* USEM4 */


static int twmStringListInput(void)
{
    if (overflowlen) return (int) overflowbuff[--overflowlen];

    /*
     * return the character currently pointed to
     */
    if (currentString) {
	unsigned int c = (unsigned int) *currentString++;

	if (c) return c;		/* if non-nul char */
	currentString = *++stringListSource;  /* advance to next bol */
	return '\n';			/* but say that we hit last eol */
    }
    return 0;				/* eof */
}


/***********************************************************************
 *
 *  Procedure:
 *	twmUnput - redefinition of the lex unput routine
 *
 *  Inputs:
 *	c	- the character to push back onto the input stream
 *
 ***********************************************************************
 */

void twmUnput (int c)
{
    if (overflowlen < sizeof overflowbuff) {
	overflowbuff[overflowlen++] = (char) c;
    } else {
	twmrc_error_prefix ();
	fprintf (stderr, "unable to unput character (%c)\n",
		 c);
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	TwmOutput - redefinition of the lex output routine
 *
 *  Inputs:
 *	c	- the character to print
 *
 ***********************************************************************
 */

void TwmOutput(int c)
{
    putchar(c);
}


/**********************************************************************
 *
 *  Parsing table and routines
 *
 ***********************************************************************/

typedef struct _TwmKeyword {
    char *name;
    int value;
    int subnum;
} TwmKeyword;

#define kw0_NoDefaults			1
#define kw0_AutoRelativeResize		2
#define kw0_ForceIcons			3
#define kw0_NoIconManagers		4
#define kw0_InterpolateMenuColors	6
#define kw0_NoVersion			7
#define kw0_SortIconManager		8
#define kw0_NoGrabServer		9
#define kw0_NoMenuShadows		10
#define kw0_NoRaiseOnMove		11
#define kw0_NoRaiseOnResize		12
#define kw0_NoRaiseOnDeiconify		13
#define kw0_DontMoveOff			14
#define kw0_NoBackingStore		15
#define kw0_NoSaveUnders		16
#define kw0_RestartPreviousState	17
#define kw0_ClientBorderWidth		18
#define kw0_NoTitleFocus		19
#define kw0_DecorateTransients		21
#define kw0_ShowIconManager		22
#define kw0_NoCaseSensitive		23
#define kw0_NoRaiseOnWarp		24
#define kw0_WarpUnmapped		25
#define kw0_ShowWorkspaceManager	27
#define kw0_StartInMapState		28
#define kw0_NoShowOccupyAll		29
#define kw0_AutoOccupy			30
#define kw0_TransientHasOccupation	31
#define kw0_DontPaintRootWindow		32
#define kw0_Use3DMenus			33
#define kw0_Use3DTitles			34
#define kw0_Use3DIconManagers		35
#define kw0_Use3DBorders		36
#define kw0_SunkFocusWindowTitle	37
#define kw0_BeNiceToColormap		38
#define kw0_WarpRingOnScreen		40
#define kw0_NoIconManagerFocus		41
#define kw0_StayUpMenus			42
#define kw0_ClickToFocus		43
#define kw0_BorderResizeCursors		44
#define kw0_ReallyMoveInWorkspaceManager 45
#define kw0_ShowWinWhenMovingInWmgr	46
#define kw0_Use3DWMap			47
#define kw0_ReverseCurrentWorkspace	48
#define kw0_DontWarpCursorInWMap	49
#define kw0_CenterFeedbackWindow	50
#define kw0_WarpToDefaultMenuEntry	51
#define kw0_ShrinkIconTitles		52
#define kw0_AutoRaiseIcons		53
#define kw0_use3DIconBorders		54
#define kw0_UseSunkTitlePixmap          55
#define kw0_ShortAllWindowsMenus	56
#define kw0_RaiseWhenAutoUnSqueeze	57
#define kw0_RaiseOnClick		58
#define kw0_IgnoreLockModifier		59
#define kw0_AutoFocusToTransients       60 /* kai */
#define kw0_PackNewWindows		61
#define kw0_IgnoreCaseInMenuSelection	62
#define kw0_SloppyFocus                 63
#define kw0_NoImagesInWorkSpaceManager  64
#define kw0_NoWarpToMenuTitle           65
#define kw0_SaveWorkspaceFocus          66 /* blais */
#define kw0_RaiseOnWarp			67

#define kws_UsePPosition		1
#define kws_IconFont			2
#define kws_ResizeFont			3
#define kws_MenuFont			4
#define kws_TitleFont			5
#define kws_IconManagerFont		6
#define kws_UnknownIcon			7
#define kws_IconDirectory		8
#define kws_MaxWindowSize		9
#define kws_PixmapDirectory		10
/* RandomPlacement moved because it's now a string string keyword */
#define kws_IconJustification		12
#define kws_TitleJustification		13
#define kws_IconRegionJustification	14
#define kws_IconRegionAlignement	15
#ifdef SOUNDS
#define kws_SoundHost			16
#endif
#define kws_WMgrButtonStyle		17
#define kws_WorkSpaceFont               18
#define kws_IconifyStyle                19

#define kwss_RandomPlacement		1

#define kwn_ConstrainedMoveTime		1
#define kwn_MoveDelta			2
#define kwn_XorValue			3
#define kwn_FramePadding		4
#define kwn_TitlePadding		5
#define kwn_ButtonIndent		6
#define kwn_BorderWidth			7
#define kwn_IconBorderWidth		8
#define kwn_TitleButtonBorderWidth	9
#define kwn_RaiseDelay			10
#define kwn_TransientOnTop		11
#define kwn_OpaqueMoveThreshold		12
#define kwn_OpaqueResizeThreshold	13
#define kwn_WMgrVertButtonIndent	14
#define kwn_WMgrHorizButtonIndent	15
#define kwn_ClearShadowContrast		16
#define kwn_DarkShadowContrast		17
#define kwn_WMgrButtonShadowDepth	18
#define kwn_MaxIconTitleWidth		19
#define kwn_AnimationSpeed		20
#define kwn_ThreeDBorderWidth		21
#define kwn_MoveOffResistance		22
#define kwn_BorderShadowDepth		23
#define kwn_TitleShadowDepth		24
#define kwn_TitleButtonShadowDepth	25
#define kwn_MenuShadowDepth		26
#define kwn_IconManagerShadowDepth	27
#define kwn_MovePackResistance		28
#define kwn_XMoveGrid			29
#define kwn_YMoveGrid			30
#define kwn_OpenWindowTimeout		31
#define kwn_RaiseOnClickButton		32

#define kwn_BorderTop			33
#define kwn_BorderBottom		34
#define kwn_BorderLeft			35
#define kwn_BorderRight			36

#define kwcl_BorderColor		1
#define kwcl_IconManagerHighlight	2
#define kwcl_BorderTileForeground	3
#define kwcl_BorderTileBackground	4
#define kwcl_TitleForeground		5
#define kwcl_TitleBackground		6
#define kwcl_IconForeground		7
#define kwcl_IconBackground		8
#define kwcl_IconBorderColor		9
#define kwcl_IconManagerForeground	10
#define kwcl_IconManagerBackground	11
#define kwcl_MapWindowBackground	12
#define kwcl_MapWindowForeground	13

#define kwc_DefaultForeground		1
#define kwc_DefaultBackground		2
#define kwc_MenuForeground		3
#define kwc_MenuBackground		4
#define kwc_MenuTitleForeground		5
#define kwc_MenuTitleBackground		6
#define kwc_MenuShadowColor		7

/*
 * The following is sorted alphabetically according to name (which must be
 * in lowercase and only contain the letters a-z).  It is fed to a binary
 * search to parse keywords.
 */
static TwmKeyword keytable[] = { 
    { "a",			ALTER, 0 },
    { "all",			ALL, 0 },
    { "alter",			ALTER, 0 },
    { "alwaysontop",		ALWAYS_ON_TOP, 0 },
    { "alwaysshowwindowwhenmovingfromworkspacemanager", KEYWORD, kw0_ShowWinWhenMovingInWmgr },
    { "alwayssqueezetogravity",	ALWAYSSQUEEZETOGRAVITY, 0 },
    { "animationspeed",		NKEYWORD, kwn_AnimationSpeed },
    { "autofocustotransients",  KEYWORD, kw0_AutoFocusToTransients }, /* kai */
    { "autolower",		AUTO_LOWER, 0 },
    { "autooccupy",		KEYWORD, kw0_AutoOccupy },
    { "autoraise",		AUTO_RAISE, 0 },
    { "autoraiseicons",		KEYWORD, kw0_AutoRaiseIcons },
    { "autorelativeresize",	KEYWORD, kw0_AutoRelativeResize },
    { "autosqueeze",		AUTOSQUEEZE, 0 },
    { "benicetocolormap",	KEYWORD, kw0_BeNiceToColormap },
    { "borderbottom",		NKEYWORD, kwn_BorderBottom },
    { "bordercolor",		CLKEYWORD, kwcl_BorderColor },
    { "borderleft",		NKEYWORD, kwn_BorderLeft },
    { "borderresizecursors",	KEYWORD, kw0_BorderResizeCursors },
    { "borderright",		NKEYWORD, kwn_BorderRight },
    { "bordershadowdepth",	NKEYWORD, kwn_BorderShadowDepth },
    { "bordertilebackground",	CLKEYWORD, kwcl_BorderTileBackground },
    { "bordertileforeground",	CLKEYWORD, kwcl_BorderTileForeground },
    { "bordertop",		NKEYWORD, kwn_BorderTop },
    { "borderwidth",		NKEYWORD, kwn_BorderWidth },
    { "button",			BUTTON, 0 },
    { "buttonindent",		NKEYWORD, kwn_ButtonIndent },
    { "c",			CONTROL, 0 },
    { "center",			JKEYWORD, J_CENTER },
    { "centerfeedbackwindow",	KEYWORD, kw0_CenterFeedbackWindow },
    { "changeworkspacefunction", CHANGE_WORKSPACE_FUNCTION, 0 },
    { "clearshadowcontrast",	NKEYWORD, kwn_ClearShadowContrast },
    { "clicktofocus",		KEYWORD, kw0_ClickToFocus },
    { "clientborderwidth",	KEYWORD, kw0_ClientBorderWidth },
    { "color",			COLOR, 0 },
    { "constrainedmovetime",	NKEYWORD, kwn_ConstrainedMoveTime },
    { "control",		CONTROL, 0 },
    { "cursors",		CURSORS, 0 },
    { "darkshadowcontrast",	NKEYWORD, kwn_DarkShadowContrast },
    { "decoratetransients",	KEYWORD, kw0_DecorateTransients },
    { "defaultbackground",	CKEYWORD, kwc_DefaultBackground },
    { "defaultforeground",	CKEYWORD, kwc_DefaultForeground },
    { "defaultfunction",	DEFAULT_FUNCTION, 0 },
    { "deiconifyfunction",	DEICONIFY_FUNCTION, 0 },
    { "destroy",		KILL, 0 },
    { "donticonifybyunmapping",	DONT_ICONIFY_BY_UNMAPPING, 0 },
    { "dontmoveoff",		KEYWORD, kw0_DontMoveOff },
    { "dontpaintrootwindow",	KEYWORD, kw0_DontPaintRootWindow },
    { "dontsave",		DONT_SAVE, 0 },
    { "dontsetinactive",	DONTSETINACTIVE, 0 },
    { "dontsqueezetitle",	DONT_SQUEEZE_TITLE, 0 },
    { "donttoggleworkspacemanagerstate", DONTTOGGLEWORKSPACEMANAGERSTATE, 0 },
    { "dontwarpcursorinwmap",	KEYWORD, kw0_DontWarpCursorInWMap },
    { "east",			DKEYWORD, D_EAST },
    { "f",			FRAME, 0 },
    { "f.addtoworkspace",	FSKEYWORD, F_ADDTOWORKSPACE },
    { "f.adoptwindow",		FKEYWORD, F_ADOPTWINDOW },
    { "f.altcontext",		FKEYWORD, F_ALTCONTEXT },
    { "f.altkeymap",		FSKEYWORD, F_ALTKEYMAP },
    { "f.autolower",		FKEYWORD, F_AUTOLOWER },
    { "f.autoraise",		FKEYWORD, F_AUTORAISE },
    { "f.backiconmgr",		FKEYWORD, F_BACKICONMGR },
    { "f.backmapiconmgr",	FKEYWORD, F_BACKMAPICONMGR },
    { "f.beep",			FKEYWORD, F_BEEP },
    { "f.bottomzoom",		FKEYWORD, F_BOTTOMZOOM },
    { "f.changesize",           FSKEYWORD, F_CHANGESIZE },
    { "f.circledown",		FKEYWORD, F_CIRCLEDOWN },
    { "f.circleup",		FKEYWORD, F_CIRCLEUP },
    { "f.colormap",		FSKEYWORD, F_COLORMAP },
    { "f.cut",			FSKEYWORD, F_CUT },
    { "f.cutfile",		FKEYWORD, F_CUTFILE },
    { "f.deiconify",		FKEYWORD, F_DEICONIFY },
    { "f.delete",		FKEYWORD, F_DELETE },
    { "f.deleteordestroy",	FKEYWORD, F_DELETEORDESTROY },
    { "f.deltastop",		FKEYWORD, F_DELTASTOP },
    { "f.destroy",		FKEYWORD, F_DESTROY },
    { "f.downiconmgr",		FKEYWORD, F_DOWNICONMGR },
    { "f.downworkspace",	FKEYWORD, F_DOWNWORKSPACE },
    { "f.exec",			FSKEYWORD, F_EXEC },
    { "f.file",			FSKEYWORD, F_FILE },
    { "f.fill",			FSKEYWORD, F_FILL },
    { "f.fittocontent",		FKEYWORD, F_FITTOCONTENT },
    { "f.focus",		FKEYWORD, F_FOCUS },
    { "f.forcemove",		FKEYWORD, F_FORCEMOVE },
    { "f.forwiconmgr",		FKEYWORD, F_FORWICONMGR },
    { "f.forwmapiconmgr",	FKEYWORD, F_FORWMAPICONMGR },
    { "f.fullzoom",		FKEYWORD, F_FULLZOOM },
    { "f.function",		FSKEYWORD, F_FUNCTION },
    { "f.gotoworkspace",	FSKEYWORD, F_GOTOWORKSPACE },
    { "f.hbzoom",		FKEYWORD, F_BOTTOMZOOM },
    { "f.hideiconmgr",		FKEYWORD, F_HIDELIST },
    { "f.hideworkspacemgr",	FKEYWORD, F_HIDEWORKMGR },
    { "f.horizoom",		FKEYWORD, F_HORIZOOM },
    { "f.htzoom",		FKEYWORD, F_TOPZOOM },
    { "f.hypermove",		FKEYWORD, F_HYPERMOVE },
    { "f.hzoom",		FKEYWORD, F_HORIZOOM },
    { "f.iconify",		FKEYWORD, F_ICONIFY },
    { "f.identify",		FKEYWORD, F_IDENTIFY },
    { "f.initsize",		FKEYWORD, F_INITSIZE },
    { "f.jumpdown",		FSKEYWORD, F_JUMPDOWN },
    { "f.jumpleft",		FSKEYWORD, F_JUMPLEFT },
    { "f.jumpright",		FSKEYWORD, F_JUMPRIGHT },
    { "f.jumpup",		FSKEYWORD, F_JUMPUP },
    { "f.lefticonmgr",		FKEYWORD, F_LEFTICONMGR },
    { "f.leftworkspace",	FKEYWORD, F_LEFTWORKSPACE },
    { "f.leftzoom",		FKEYWORD, F_LEFTZOOM },
    { "f.lower",		FKEYWORD, F_LOWER },
    { "f.menu",			FSKEYWORD, F_MENU },
    { "f.move",			FKEYWORD, F_MOVE },
    { "f.movemenu",		FKEYWORD, F_MOVEMENU },
    { "f.movepack",		FKEYWORD, F_MOVEPACK },
    { "f.movepush",		FKEYWORD, F_MOVEPUSH },
    { "f.moveresize",		FSKEYWORD, F_MOVERESIZE },
    { "f.movetitlebar",		FKEYWORD, F_MOVETITLEBAR },
    { "f.movetonextworkspace",  FKEYWORD, F_MOVETONEXTWORKSPACE },
    { "f.movetonextworkspaceandfollow",  FKEYWORD, F_MOVETONEXTWORKSPACEANDFOLLOW },
    { "f.movetoprevworkspace",  FKEYWORD, F_MOVETOPREVWORKSPACE },
    { "f.movetoprevworkspaceandfollow",  FKEYWORD, F_MOVETOPREVWORKSPACEANDFOLLOW },
    { "f.nexticonmgr",		FKEYWORD, F_NEXTICONMGR },
    { "f.nextworkspace",	FKEYWORD, F_NEXTWORKSPACE },
    { "f.nop",			FKEYWORD, F_NOP },
    { "f.occupy",		FKEYWORD, F_OCCUPY },
    { "f.occupyall",		FKEYWORD, F_OCCUPYALL },
    { "f.pack",			FSKEYWORD, F_PACK },
    { "f.pin",			FKEYWORD, F_PIN },
    { "f.previconmgr",		FKEYWORD, F_PREVICONMGR },
    { "f.prevworkspace",	FKEYWORD, F_PREVWORKSPACE },
    { "f.quit",			FKEYWORD, F_QUIT },
    { "f.raise",		FKEYWORD, F_RAISE },
    { "f.raiseicons",		FKEYWORD, F_RAISEICONS },
    { "f.raiselower",		FKEYWORD, F_RAISELOWER },
    { "f.refresh",		FKEYWORD, F_REFRESH },
    { "f.removefromworkspace",	FSKEYWORD, F_REMOVEFROMWORKSPACE },
#ifdef SOUNDS
    { "f.rereadsounds",		FKEYWORD, F_REREADSOUNDS },
#endif
    { "f.resize",		FKEYWORD, F_RESIZE },
    { "f.restart",		FKEYWORD, F_RESTART },
    { "f.restoregeometry",	FKEYWORD, F_RESTOREGEOMETRY },
    { "f.righticonmgr",		FKEYWORD, F_RIGHTICONMGR },
    { "f.rightworkspace",	FKEYWORD, F_RIGHTWORKSPACE },
    { "f.rightzoom",		FKEYWORD, F_RIGHTZOOM },
    { "f.ring",			FKEYWORD, F_RING },
    { "f.savegeometry",		FKEYWORD, F_SAVEGEOMETRY },
    { "f.saveyourself",		FKEYWORD, F_SAVEYOURSELF },
    { "f.separator",		FKEYWORD, F_SEPARATOR },
    { "f.setbuttonsstate",	FKEYWORD, F_SETBUTTONSTATE },
    { "f.setmapstate",		FKEYWORD, F_SETMAPSTATE },
    { "f.showbackground",      	FKEYWORD, F_SHOWBGRD },
    { "f.showiconmgr",		FKEYWORD, F_SHOWLIST },
    { "f.showworkspacemgr",	FKEYWORD, F_SHOWWORKMGR },
    { "f.slowdownanimation",	FKEYWORD, F_SLOWDOWNANIMATION },
    { "f.sorticonmgr",		FKEYWORD, F_SORTICONMGR },
    { "f.source",		FSKEYWORD, F_BEEP },  /* XXX - don't work */
    { "f.speedupanimation",	FKEYWORD, F_SPEEDUPANIMATION },
    { "f.squeeze",		FKEYWORD, F_SQUEEZE },
    { "f.startanimation",	FKEYWORD, F_STARTANIMATION },
    { "f.stopanimation",	FKEYWORD, F_STOPANIMATION },
    { "f.title",		FKEYWORD, F_TITLE },
    { "f.toggleoccupation",	FSKEYWORD, F_TOGGLEOCCUPATION },
#ifdef SOUNDS
    { "f.togglesound",		FKEYWORD, F_TOGGLESOUND },
#endif
    { "f.togglestate",		FKEYWORD, F_TOGGLESTATE },
    { "f.toggleworkspacemgr",	FKEYWORD, F_TOGGLEWORKMGR },
    { "f.topzoom",		FKEYWORD, F_TOPZOOM },
    { "f.trace",		FSKEYWORD, F_TRACE },
    { "f.twmrc",		FKEYWORD, F_RESTART },
    { "f.unfocus",		FKEYWORD, F_UNFOCUS },
    { "f.upiconmgr",		FKEYWORD, F_UPICONMGR },
    { "f.upworkspace",		FKEYWORD, F_UPWORKSPACE },
    { "f.vanish",		FKEYWORD, F_VANISH },
    { "f.version",		FKEYWORD, F_VERSION },
    { "f.vlzoom",		FKEYWORD, F_LEFTZOOM },
    { "f.vrzoom",		FKEYWORD, F_RIGHTZOOM },
    { "f.warphere",		FSKEYWORD, F_WARPHERE },
    { "f.warpring",		FSKEYWORD, F_WARPRING },
    { "f.warpto",		FSKEYWORD, F_WARPTO },
    { "f.warptoiconmgr",	FSKEYWORD, F_WARPTOICONMGR },
    { "f.warptoscreen",		FSKEYWORD, F_WARPTOSCREEN },
    { "f.winrefresh",		FKEYWORD, F_WINREFRESH },
    { "f.zoom",			FKEYWORD, F_ZOOM },
    { "forceicons",		KEYWORD, kw0_ForceIcons },
    { "frame",			FRAME, 0 },
    { "framepadding",		NKEYWORD, kwn_FramePadding },
    { "function",		FUNCTION, 0 },
    { "i",			ICON, 0 },
    { "icon",			ICON, 0 },
    { "iconbackground",		CLKEYWORD, kwcl_IconBackground },
    { "iconbordercolor",	CLKEYWORD, kwcl_IconBorderColor },
    { "iconborderwidth",	NKEYWORD, kwn_IconBorderWidth },
    { "icondirectory",		SKEYWORD, kws_IconDirectory },
    { "iconfont",		SKEYWORD, kws_IconFont },
    { "iconforeground",		CLKEYWORD, kwcl_IconForeground },
    { "iconifybyunmapping",	ICONIFY_BY_UNMAPPING, 0 },
    { "iconifyfunction",	ICONIFY_FUNCTION, 0 },
    { "iconifystyle",		SKEYWORD, kws_IconifyStyle },
    { "iconjustification",	SKEYWORD, kws_IconJustification },
    { "iconmanagerbackground",	CLKEYWORD, kwcl_IconManagerBackground },
    { "iconmanagerdontshow",	ICONMGR_NOSHOW, 0 },
    { "iconmanagerfont",	SKEYWORD, kws_IconManagerFont },
    { "iconmanagerforeground",	CLKEYWORD, kwcl_IconManagerForeground },
    { "iconmanagergeometry",	ICONMGR_GEOMETRY, 0 },
    { "iconmanagerhighlight",	CLKEYWORD, kwcl_IconManagerHighlight },
    { "iconmanagers",		ICONMGRS, 0 },
    { "iconmanagershadowdepth",	NKEYWORD, kwn_IconManagerShadowDepth },
    { "iconmanagershow",	ICONMGR_SHOW, 0 },
    { "iconmenudontshow",	ICONMENU_DONTSHOW, 0 },
    { "iconmgr",		ICONMGR, 0 },
    { "iconregion",		ICON_REGION, 0 },
    { "iconregionalignement",	SKEYWORD, kws_IconRegionAlignement },
    { "iconregionjustification",SKEYWORD, kws_IconRegionJustification },
    { "icons",			ICONS, 0 },
    { "ignorecaseinmenuselection",	KEYWORD, kw0_IgnoreCaseInMenuSelection },
    { "ignorelockmodifier",	KEYWORD, kw0_IgnoreLockModifier },
    { "ignoremodifier",		IGNOREMODIFIER, 0 },
    { "ignoretransient",	IGNORE_TRANSIENT, 0 },
    { "interpolatemenucolors",	KEYWORD, kw0_InterpolateMenuColors },
    { "l",			LOCK, 0 },
    { "left",			JKEYWORD, J_LEFT },
    { "lefttitlebutton",	LEFT_TITLEBUTTON, 0 },
    { "lock",			LOCK, 0 },
    { "m",			META, 0 },
    { "maketitle",		MAKE_TITLE, 0 },
    { "mapwindowbackground",	CLKEYWORD, kwcl_MapWindowBackground },
    { "mapwindowcurrentworkspace", MAPWINDOWCURRENTWORKSPACE, 0},
    { "mapwindowdefaultworkspace", MAPWINDOWDEFAULTWORKSPACE, 0},
    { "mapwindowforeground",	CLKEYWORD, kwcl_MapWindowForeground },
    { "maxicontitlewidth",	NKEYWORD, kwn_MaxIconTitleWidth },
    { "maxwindowsize",		SKEYWORD, kws_MaxWindowSize },
    { "menu",			MENU, 0 },
    { "menubackground",		CKEYWORD, kwc_MenuBackground },
    { "menufont",		SKEYWORD, kws_MenuFont },
    { "menuforeground",		CKEYWORD, kwc_MenuForeground },
    { "menushadowcolor",	CKEYWORD, kwc_MenuShadowColor },
    { "menushadowdepth",	NKEYWORD, kwn_MenuShadowDepth },
    { "menutitlebackground",	CKEYWORD, kwc_MenuTitleBackground },
    { "menutitleforeground",	CKEYWORD, kwc_MenuTitleForeground },
    { "meta",			META, 0 },
    { "mod",			META, 0 },  /* fake it */
    { "monochrome",		MONOCHROME, 0 },
    { "move",			MOVE, 0 },
    { "movedelta",		NKEYWORD, kwn_MoveDelta },
    { "moveoffresistance",	NKEYWORD, kwn_MoveOffResistance },
    { "movepackresistance",	NKEYWORD, kwn_MovePackResistance },
    { "nobackingstore",		KEYWORD, kw0_NoBackingStore },
    { "noborder",		NO_BORDER, 0 },
    { "nocasesensitive",	KEYWORD, kw0_NoCaseSensitive },
    { "nodefaults",		KEYWORD, kw0_NoDefaults },
    { "nograbserver",		KEYWORD, kw0_NoGrabServer },
    { "nohighlight",		NO_HILITE, 0 },
    { "noiconmanagerfocus",	KEYWORD, kw0_NoIconManagerFocus },
    { "noiconmanagers",		KEYWORD, kw0_NoIconManagers },
    { "noicontitle",		NO_ICON_TITLE, 0  },
    { "noimagesinworkspacemanager", KEYWORD, kw0_NoImagesInWorkSpaceManager },
    { "nomenushadows",		KEYWORD, kw0_NoMenuShadows },
    { "noopaquemove",		NOOPAQUEMOVE, 0 },
    { "noopaqueresize",		NOOPAQUERESIZE, 0 },
    { "noraiseondeiconify",	KEYWORD, kw0_NoRaiseOnDeiconify },
    { "noraiseonmove",		KEYWORD, kw0_NoRaiseOnMove },
    { "noraiseonresize",	KEYWORD, kw0_NoRaiseOnResize },
    { "noraiseonwarp",		KEYWORD, kw0_NoRaiseOnWarp },
    { "north",			DKEYWORD, D_NORTH },
    { "nosaveunders",		KEYWORD, kw0_NoSaveUnders },
    { "noshowoccupyall",	KEYWORD, kw0_NoShowOccupyAll },
    { "nostackmode",		NO_STACKMODE, 0 },
    { "notitle",		NO_TITLE, 0 },
    { "notitlefocus",		KEYWORD, kw0_NoTitleFocus },
    { "notitlehighlight",	NO_TITLE_HILITE, 0 },
    { "noversion",		KEYWORD, kw0_NoVersion },
    { "nowarptomenutitle",      KEYWORD, kw0_NoWarpToMenuTitle },
    { "occupy",			OCCUPYLIST, 0 },
    { "occupyall",		OCCUPYALL, 0 },
    { "opaquemove",		OPAQUEMOVE, 0 },
    { "opaquemovethreshold",	NKEYWORD, kwn_OpaqueMoveThreshold },
    { "opaqueresize",		OPAQUERESIZE, 0 },
    { "opaqueresizethreshold",	NKEYWORD, kwn_OpaqueResizeThreshold },
    { "openwindowtimeout",	NKEYWORD, kwn_OpenWindowTimeout },
    { "packnewwindows",		KEYWORD, kw0_PackNewWindows },
    { "pixmapdirectory",	SKEYWORD, kws_PixmapDirectory },
    { "pixmaps",		PIXMAPS, 0 },
    { "r",			ROOT, 0 },
    { "raisedelay",		NKEYWORD, kwn_RaiseDelay },
    { "raiseonclick",		KEYWORD, kw0_RaiseOnClick },
    { "raiseonclickbutton",	NKEYWORD, kwn_RaiseOnClickButton },
    { "raiseonwarp",		KEYWORD, kw0_RaiseOnWarp },
    { "raisewhenautounsqueeze",	KEYWORD, kw0_RaiseWhenAutoUnSqueeze },
    { "randomplacement",	SSKEYWORD, kwss_RandomPlacement },
    { "reallymoveinworkspacemanager",	KEYWORD, kw0_ReallyMoveInWorkspaceManager },
    { "resize",			RESIZE, 0 },
    { "resizefont",		SKEYWORD, kws_ResizeFont },
    { "restartpreviousstate",	KEYWORD, kw0_RestartPreviousState },
    { "reversecurrentworkspace",KEYWORD, kw0_ReverseCurrentWorkspace },
    { "right",			JKEYWORD, J_RIGHT },
    { "righttitlebutton",	RIGHT_TITLEBUTTON, 0 },
    { "root",			ROOT, 0 },
    { "s",			SHIFT, 0 },
    { "savecolor",              SAVECOLOR, 0},
    { "saveworkspacefocus",     KEYWORD, kw0_SaveWorkspaceFocus },
    { "schrinkicontitles",	KEYWORD, kw0_ShrinkIconTitles },
    { "select",			SELECT, 0 },
    { "shift",			SHIFT, 0 },
    { "shortallwindowsmenus",	KEYWORD, kw0_ShortAllWindowsMenus },
    { "showiconmanager",	KEYWORD, kw0_ShowIconManager },
    { "showworkspacemanager",	KEYWORD, kw0_ShowWorkspaceManager },
    { "shrinkicontitles",	KEYWORD, kw0_ShrinkIconTitles },
    { "sloppyfocus",            KEYWORD, kw0_SloppyFocus },
    { "sorticonmanager",	KEYWORD, kw0_SortIconManager },
#ifdef SOUNDS
    { "soundhost",		SKEYWORD, kws_SoundHost },
#endif
    { "south",			DKEYWORD, D_SOUTH },
    { "squeezetitle",		SQUEEZE_TITLE, 0 },
    { "starticonified",		START_ICONIFIED, 0 },
    { "startinmapstate",	KEYWORD, kw0_StartInMapState },
    { "startsqueezed",		STARTSQUEEZED, 0 },
    { "stayupmenus",		KEYWORD, kw0_StayUpMenus },
    { "sunkfocuswindowtitle",	KEYWORD, kw0_SunkFocusWindowTitle },
    { "t",			TITLE, 0 },
    { "threedborderwidth",	NKEYWORD, kwn_ThreeDBorderWidth },
    { "title",			TITLE, 0 },
    { "titlebackground",	CLKEYWORD, kwcl_TitleBackground },
    { "titlebuttonborderwidth",	NKEYWORD, kwn_TitleButtonBorderWidth },
    { "titlebuttonshadowdepth",	NKEYWORD, kwn_TitleButtonShadowDepth },
    { "titlefont",		SKEYWORD, kws_TitleFont },
    { "titleforeground",	CLKEYWORD, kwcl_TitleForeground },
    { "titlehighlight",		TITLE_HILITE, 0 },
    { "titlejustification",	SKEYWORD, kws_TitleJustification },
    { "titlepadding",		NKEYWORD, kwn_TitlePadding },
    { "titleshadowdepth",	NKEYWORD, kwn_TitleShadowDepth },
    { "transienthasoccupation",	KEYWORD, kw0_TransientHasOccupation },
    { "transientontop",		NKEYWORD, kwn_TransientOnTop },
    { "unknownicon",		SKEYWORD, kws_UnknownIcon },
    { "unmapbymovingfaraway",	UNMAPBYMOVINGFARAWAY, 0 },
    { "usepposition",		SKEYWORD, kws_UsePPosition },
    { "usesunktitlepixmap",     KEYWORD, kw0_UseSunkTitlePixmap },
    { "usethreedborders",	KEYWORD, kw0_Use3DBorders },
    { "usethreediconborders",	KEYWORD, kw0_use3DIconBorders },
    { "usethreediconmanagers",	KEYWORD, kw0_Use3DIconManagers },
    { "usethreedmenus",		KEYWORD, kw0_Use3DMenus },
    { "usethreedtitles",	KEYWORD, kw0_Use3DTitles },
    { "usethreedwmap",		KEYWORD, kw0_Use3DWMap },
    { "virtualscreens",         VIRTUAL_SCREENS, 0 },
    { "w",			WINDOW, 0 },
    { "wait",			WAITC, 0 },
    { "warpcursor",		WARP_CURSOR, 0 },
    { "warpringonscreen",	KEYWORD, kw0_WarpRingOnScreen },
    { "warptodefaultmenuentry",	KEYWORD, kw0_WarpToDefaultMenuEntry },
    { "warpunmapped",		KEYWORD, kw0_WarpUnmapped },
    { "west",			DKEYWORD, D_WEST },
    { "window",			WINDOW, 0 },
    { "windowbox",		WINDOW_BOX, 0 },
    { "windowfunction",		WINDOW_FUNCTION, 0 },
    { "windowgeometries",	WINDOW_GEOMETRIES, 0 },
    { "windowregion",		WINDOW_REGION, 0 },
    { "windowring",		WINDOW_RING, 0 },
    { "windowringexclude",      WINDOW_RING_EXCLUDE, 0},
    { "wmgrbuttonshadowdepth",	NKEYWORD, kwn_WMgrButtonShadowDepth },
    { "wmgrbuttonstyle",	SKEYWORD, kws_WMgrButtonStyle },
    { "wmgrhorizbuttonindent",	NKEYWORD, kwn_WMgrHorizButtonIndent },
    { "wmgrvertbuttonindent",	NKEYWORD, kwn_WMgrVertButtonIndent },
    { "workspace", 		WORKSPACE, 0 },
    { "workspacefont",          SKEYWORD, kws_WorkSpaceFont },
    { "workspacemanagergeometry", WORKSPCMGR_GEOMETRY, 0 },
    { "workspaces",             WORKSPACES, 0},
    { "xmovegrid",		NKEYWORD, kwn_XMoveGrid },
    { "xorvalue",		NKEYWORD, kwn_XorValue },
    { "xpmicondirectory",	SKEYWORD, kws_PixmapDirectory },
    { "ymovegrid",		NKEYWORD, kwn_YMoveGrid },
    { "zoom",			ZOOM, 0 },
};

static int numkeywords = (sizeof(keytable)/sizeof(keytable[0]));

int parse_keyword (char *s, int *nump)
{
    register int lower = 0, upper = numkeywords - 1;

    XmuCopyISOLatin1Lowered (s, s);
    while (lower <= upper) {
        int middle = (lower + upper) / 2;
	TwmKeyword *p = &keytable[middle];
        int res = strcmp (p->name, s);

        if (res < 0) {
            lower = middle + 1;
        } else if (res == 0) {
	    *nump = p->subnum;
            return p->value;
        } else {
            upper = middle - 1;
        }
    }
    return ERRORTOKEN;
}



/*
 * action routines called by grammar
 */

int do_single_keyword (int keyword)
{
    switch (keyword) {
      case kw0_NoDefaults:
	Scr->NoDefaults = TRUE;
	return 1;

      case kw0_AutoRelativeResize:
	Scr->AutoRelativeResize = TRUE;
	return 1;

      case kw0_ForceIcons:
	if (Scr->FirstTime) Scr->ForceIcon = TRUE;
	return 1;

      case kw0_NoIconManagers:
	Scr->NoIconManagers = TRUE;
	return 1;

      case kw0_InterpolateMenuColors:
	if (Scr->FirstTime) Scr->InterpolateMenuColors = TRUE;
	return 1;

      case kw0_NoVersion:
	/* obsolete */
	return 1;

      case kw0_SortIconManager:
	if (Scr->FirstTime) Scr->SortIconMgr = TRUE;
	return 1;

      case kw0_NoGrabServer:
	Scr->NoGrabServer = TRUE;
	return 1;

      case kw0_NoMenuShadows:
	if (Scr->FirstTime) Scr->Shadow = FALSE;
	return 1;

      case kw0_NoRaiseOnMove:
	if (Scr->FirstTime) Scr->NoRaiseMove = TRUE;
	return 1;

      case kw0_NoRaiseOnResize:
	if (Scr->FirstTime) Scr->NoRaiseResize = TRUE;
	return 1;

      case kw0_NoRaiseOnDeiconify:
	if (Scr->FirstTime) Scr->NoRaiseDeicon = TRUE;
	return 1;

      case kw0_DontMoveOff:
	Scr->DontMoveOff = TRUE;
	return 1;

      case kw0_NoBackingStore:
	Scr->BackingStore = FALSE;
	return 1;

      case kw0_NoSaveUnders:
	Scr->SaveUnder = FALSE;
	return 1;

      case kw0_RestartPreviousState:
	RestartPreviousState = True;
	return 1;

      case kw0_ClientBorderWidth:
	if (Scr->FirstTime) Scr->ClientBorderWidth = TRUE;
	return 1;

      case kw0_NoTitleFocus:
	Scr->TitleFocus = TRUE /*FALSE*/;
	return 1;

      case kw0_DecorateTransients:
	Scr->DecorateTransients = TRUE;
	return 1;

      case kw0_ShowIconManager:
	Scr->ShowIconManager = TRUE;
	return 1;

      case kw0_ShowWorkspaceManager:
	Scr->ShowWorkspaceManager = TRUE;
	return 1;

      case kw0_StartInMapState:
	Scr->workSpaceMgr.initialstate = MAPSTATE;
	return 1;

      case kw0_NoShowOccupyAll:
	Scr->workSpaceMgr.noshowoccupyall = TRUE;
	return 1;

      case kw0_AutoOccupy:
	Scr->AutoOccupy = TRUE;
	return 1;

      case kw0_TransientHasOccupation:
	Scr->TransientHasOccupation = TRUE;
	return 1;

      case kw0_DontPaintRootWindow:
	Scr->DontPaintRootWindow = TRUE;
	return 1;

      case kw0_UseSunkTitlePixmap:
	Scr->UseSunkTitlePixmap = TRUE;
	return 1;

      case kw0_Use3DBorders:
	Scr->use3Dborders = TRUE;
	return 1;

      case kw0_Use3DIconManagers:
	Scr->use3Diconmanagers = TRUE;
	return 1;

      case kw0_Use3DMenus:
	Scr->use3Dmenus = TRUE;
	return 1;

      case kw0_Use3DTitles:
	Scr->use3Dtitles = TRUE;
	return 1;

      case kw0_Use3DWMap:
	Scr->use3Dwmap = TRUE;
	return 1;

      case kw0_SunkFocusWindowTitle:
	Scr->SunkFocusWindowTitle = TRUE;
	return 1;

      case kw0_BeNiceToColormap:
	Scr->BeNiceToColormap = TRUE;
	return 1;

      case kw0_BorderResizeCursors:
	Scr->BorderCursors = TRUE;
	return 1;

      case kw0_NoCaseSensitive:
	Scr->CaseSensitive = FALSE;
	return 1;

      case kw0_NoRaiseOnWarp:
	Scr->RaiseOnWarp = FALSE;
	return 1;

      case kw0_RaiseOnWarp:
	Scr->RaiseOnWarp = TRUE;
	return 1;

      case kw0_WarpUnmapped:
	Scr->WarpUnmapped = TRUE;
	return 1;

      case kw0_WarpRingOnScreen:
	Scr->WarpRingAnyWhere = FALSE;
	return 1;

      case kw0_NoIconManagerFocus:
	Scr->IconManagerFocus = FALSE;
	return 1;

      case kw0_StayUpMenus:
	Scr->StayUpMenus = TRUE;
	return 1;

      case kw0_ClickToFocus:
	Scr->ClickToFocus = TRUE;
	return 1;

      case kw0_ReallyMoveInWorkspaceManager:
	Scr->ReallyMoveInWorkspaceManager = TRUE;
	return 1;

      case kw0_ShowWinWhenMovingInWmgr:
	Scr->ShowWinWhenMovingInWmgr = TRUE;
	return 1;

      case kw0_ReverseCurrentWorkspace:
	Scr->ReverseCurrentWorkspace = TRUE;
	return 1;

      case kw0_DontWarpCursorInWMap:
	Scr->DontWarpCursorInWMap = TRUE;
	return 1;

      case kw0_CenterFeedbackWindow:
	Scr->CenterFeedbackWindow = TRUE;
	return 1;

      case kw0_WarpToDefaultMenuEntry:
	Scr->WarpToDefaultMenuEntry = TRUE;
	return 1;

      case kw0_ShrinkIconTitles:
	Scr->ShrinkIconTitles = TRUE;
	return 1;

      case kw0_AutoRaiseIcons:
	Scr->AutoRaiseIcons = TRUE;
	return 1;

      /* kai */
      case kw0_AutoFocusToTransients:
        Scr->AutoFocusToTransients = TRUE;
        return 1;

      case kw0_use3DIconBorders:
	Scr->use3Diconborders = TRUE;
	return 1;

      case kw0_ShortAllWindowsMenus:
	Scr->ShortAllWindowsMenus = TRUE;
	return 1;

      case kw0_RaiseWhenAutoUnSqueeze:
	Scr->RaiseWhenAutoUnSqueeze = TRUE;
	return 1;

      case kw0_RaiseOnClick:
	Scr->RaiseOnClick = TRUE;
	return 1;

      case kw0_IgnoreLockModifier:
	Scr->IgnoreLockModifier = TRUE;
	return 1;

      case kw0_PackNewWindows:
	Scr->PackNewWindows = TRUE;
	return 1;

      case kw0_IgnoreCaseInMenuSelection:
	Scr->IgnoreCaseInMenuSelection = TRUE;
	return 1;

      case kw0_SloppyFocus:
	Scr->SloppyFocus = TRUE;
	return 1;

      case kw0_SaveWorkspaceFocus:
	Scr->SaveWorkspaceFocus = TRUE;
	return 1;

      case kw0_NoImagesInWorkSpaceManager:
	Scr->NoImagesInWorkSpaceManager = TRUE;
	return 1;

      case kw0_NoWarpToMenuTitle:
	Scr->NoWarpToMenuTitle = TRUE;
	return 1;

    }
    return 0;
}


int do_string_string_keyword (int keyword, char *s1, char *s2)
{
    switch (keyword) {
       case kwss_RandomPlacement:
       {
	    int rp = ParseRandomPlacement (s1);
	    if (rp < 0) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid RandomPlacement argument 1 \"%s\"\n", s1);
	    } else {
		Scr->RandomPlacement = rp;
	    }
	}
	{
	    if (strcmp (s2, "default") == 0) return 1;
	    JunkMask = XParseGeometry (s2, &JunkX, &JunkY, &JunkWidth, &JunkHeight);
#ifdef DEBUG
	    fprintf (stderr, "DEBUG:: JunkMask = %x, WidthValue = %x, HeightValue = %x\n", JunkMask, WidthValue, HeightValue);
	    fprintf (stderr, "DEBUG:: JunkX = %d, JunkY = %d\n", JunkX, JunkY);
#endif
	    if ((JunkMask & (XValue | YValue)) !=
		(XValue | YValue)) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid RandomPlacement displacement \"%s\"\n", s2);
	    } else {
		Scr->RandomDisplacementX = JunkX;
		Scr->RandomDisplacementY = JunkY;
	    }
	    return 1;
	}
    }
    return 0;
}


int do_string_keyword (int keyword, char *s)
{
    switch (keyword) {
      case kws_UsePPosition:
	{
	    int ppos = ParseUsePPosition (s);
	    if (ppos < 0) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid UsePPosition argument \"%s\"\n", s);
	    } else {
		Scr->UsePPosition = ppos;
	    }
	    return 1;
	}

      case kws_IconFont:
	if (!Scr->HaveFonts) Scr->IconFont.basename = s;
	return 1;

      case kws_ResizeFont:
	if (!Scr->HaveFonts) Scr->SizeFont.basename = s;
	return 1;

      case kws_MenuFont:
	if (!Scr->HaveFonts) Scr->MenuFont.basename = s;
	return 1;

      case kws_WorkSpaceFont:
	if (!Scr->HaveFonts) Scr->workSpaceMgr.windowFont.basename = s;
	return 1;

      case kws_TitleFont:
	if (!Scr->HaveFonts) Scr->TitleBarFont.basename = s;
	return 1;

      case kws_IconManagerFont:
	if (!Scr->HaveFonts) Scr->IconManagerFont.basename = s;
	return 1;

      case kws_UnknownIcon:
	if (Scr->FirstTime) GetUnknownIcon (s);
	return 1;

      case kws_IconDirectory:
	if (Scr->FirstTime) Scr->IconDirectory = ExpandFilePath (s);
	return 1;

      case kws_PixmapDirectory:
	if (Scr->FirstTime) Scr->PixmapDirectory = ExpandFilePath (s);
	return 1;

      case kws_MaxWindowSize:
	JunkMask = XParseGeometry (s, &JunkX, &JunkY, &JunkWidth, &JunkHeight);
	if ((JunkMask & (WidthValue | HeightValue)) != 
	    (WidthValue | HeightValue)) {
	    twmrc_error_prefix();
	    fprintf (stderr, "bad MaxWindowSize \"%s\"\n", s);
	    return 0;
	}
	if (JunkWidth == 0 || JunkHeight == 0) {
	    twmrc_error_prefix();
	    fprintf (stderr, "MaxWindowSize \"%s\" must be non-zero\n", s);
	    return 0;
	}
	Scr->MaxWindowWidth = JunkWidth;
	Scr->MaxWindowHeight = JunkHeight;
	return 1;

      case kws_IconJustification:
	{
	    int just = ParseJustification (s);

 	    if ((just < 0) || (just == J_BORDER)) {
 		twmrc_error_prefix();
 		fprintf (stderr,
 			 "ignoring invalid IconJustification argument \"%s\"\n", s);
 	    } else {
 		Scr->IconJustification = just;
 	    }
 	    return 1;
 	}
      case kws_IconRegionJustification:
	{
	    int just = ParseJustification (s);

 	    if (just < 0) {
 		twmrc_error_prefix();
 		fprintf (stderr,
 			 "ignoring invalid IconRegionJustification argument \"%s\"\n", s);
 	    } else {
 		Scr->IconRegionJustification = just;
 	    }
 	    return 1;
 	}
      case kws_IconRegionAlignement:
	{
	    int just = ParseAlignement (s);

 	    if (just < 0) {
 		twmrc_error_prefix();
 		fprintf (stderr,
 			 "ignoring invalid IconRegionAlignement argument \"%s\"\n", s);
 	    } else {
 		Scr->IconRegionAlignement = just;
 	    }
 	    return 1;
 	}

      case kws_TitleJustification:
	{
	    int just = ParseJustification (s);

 	    if ((just < 0) || (just == J_BORDER)) {
 		twmrc_error_prefix();
 		fprintf (stderr,
 			 "ignoring invalid TitleJustification argument \"%s\"\n", s);
 	    } else {
 		Scr->TitleJustification = just;
 	    }
 	    return 1;
 	}
#ifdef SOUNDS
      case kws_SoundHost:
        if (Scr->FirstTime) set_sound_host(s);
        return 1;
#endif
		
      case kws_WMgrButtonStyle:
	{
	    int style = ParseButtonStyle (s);

 	    if (style < 0) {
 		twmrc_error_prefix();
 		fprintf (stderr,
 			 "ignoring invalid WMgrButtonStyle argument \"%s\"\n", s);
 	    } else {
 		Scr->workSpaceMgr.buttonStyle = style;
 	    }
 	    return 1;
	}

      case kws_IconifyStyle:
	{
	  if (strlen (s) == 0) {
	    twmrc_error_prefix();
	    fprintf (stderr, "ignoring invalid IconifyStyle argument \"%s\"\n", s);
	  }
	  if (strcmp (s,  "default") == 0) Scr->IconifyStyle = ICONIFY_NORMAL;
	  XmuCopyISOLatin1Lowered (s, s);
	  if (strcmp (s,  "normal") == 0) Scr->IconifyStyle = ICONIFY_NORMAL;
	  if (strcmp (s,  "mosaic") == 0) Scr->IconifyStyle = ICONIFY_MOSAIC;
	  if (strcmp (s,  "zoomin") == 0) Scr->IconifyStyle = ICONIFY_ZOOMIN;
	  if (strcmp (s, "zoomout") == 0) Scr->IconifyStyle = ICONIFY_ZOOMOUT;
	  if (strcmp (s,   "sweep") == 0) Scr->IconifyStyle = ICONIFY_SWEEP;
	  return 1;
	}
    }
    return 0;
}


int do_number_keyword (int keyword, int num)
{
    switch (keyword) {
      case kwn_ConstrainedMoveTime:
	ConstrainedMoveTime = num;
	return 1;

      case kwn_MoveDelta:
	Scr->MoveDelta = num;
	return 1;

      case kwn_MoveOffResistance:
	Scr->MoveOffResistance = num;
	return 1;

      case kwn_MovePackResistance:
	if (num < 0) num = 20;
	Scr->MovePackResistance = num;
	return 1;

      case kwn_XMoveGrid:
	if (num <   1) num =   1;
	if (num > 100) num = 100;
	Scr->XMoveGrid = num;
	return 1;

      case kwn_YMoveGrid:
	if (num <   1) num =   1;
	if (num > 100) num = 100;
	Scr->YMoveGrid = num;
	return 1;

      case kwn_XorValue:
	if (Scr->FirstTime) Scr->XORvalue = num;
	return 1;

      case kwn_FramePadding:
	if (Scr->FirstTime) Scr->FramePadding = num;
	return 1;

      case kwn_TitlePadding:
	if (Scr->FirstTime) {
	    Scr->TitlePadding = num;
	}
	return 1;

      case kwn_ButtonIndent:
	if (Scr->FirstTime) Scr->ButtonIndent = num;
	return 1;

      case kwn_ThreeDBorderWidth:
	if (Scr->FirstTime) Scr->ThreeDBorderWidth = num;
	return 1;

      case kwn_BorderWidth:
	if (Scr->FirstTime) Scr->BorderWidth = num;
	return 1;

      case kwn_IconBorderWidth:
	if (Scr->FirstTime) Scr->IconBorderWidth = num;
	return 1;

      case kwn_TitleButtonBorderWidth:
	if (Scr->FirstTime) Scr->TBInfo.border = num;
	return 1;

      case kwn_RaiseDelay:
	RaiseDelay = num;
	return 1;

      case kwn_TransientOnTop:
	if (Scr->FirstTime) Scr->TransientOnTop = num;
	return 1;

      case kwn_OpaqueMoveThreshold:
	if (Scr->FirstTime) Scr->OpaqueMoveThreshold = num;
	return 1;

      case kwn_OpaqueResizeThreshold:
	if (Scr->FirstTime) Scr->OpaqueResizeThreshold = num;
	return 1;

      case kwn_WMgrVertButtonIndent:
	if (Scr->FirstTime) Scr->WMgrVertButtonIndent = num;
	if (Scr->WMgrVertButtonIndent < 0) Scr->WMgrVertButtonIndent = 0;
	Scr->workSpaceMgr.vspace = Scr->WMgrVertButtonIndent;
	Scr->workSpaceMgr.occupyWindow->vspace = Scr->WMgrVertButtonIndent;
	return 1;

      case kwn_WMgrHorizButtonIndent:
	if (Scr->FirstTime) Scr->WMgrHorizButtonIndent = num;
	if (Scr->WMgrHorizButtonIndent < 0) Scr->WMgrHorizButtonIndent = 0;
	Scr->workSpaceMgr.hspace = Scr->WMgrHorizButtonIndent;
	Scr->workSpaceMgr.occupyWindow->hspace = Scr->WMgrHorizButtonIndent;
	return 1;

      case kwn_WMgrButtonShadowDepth:
	if (Scr->FirstTime) Scr->WMgrButtonShadowDepth = num;
	if (Scr->WMgrButtonShadowDepth < 1) Scr->WMgrButtonShadowDepth = 1;
	return 1;

      case kwn_MaxIconTitleWidth:
	if (Scr->FirstTime) Scr->MaxIconTitleWidth = num;
	return 1;

      case kwn_ClearShadowContrast:
	if (Scr->FirstTime) Scr->ClearShadowContrast = num;
	if (Scr->ClearShadowContrast <   0) Scr->ClearShadowContrast =   0;
	if (Scr->ClearShadowContrast > 100) Scr->ClearShadowContrast = 100;
	return 1;

      case kwn_DarkShadowContrast:
	if (Scr->FirstTime) Scr->DarkShadowContrast = num;
	if (Scr->DarkShadowContrast <   0) Scr->DarkShadowContrast =   0;
	if (Scr->DarkShadowContrast > 100) Scr->DarkShadowContrast = 100;
	return 1;

      case kwn_AnimationSpeed:
	if (num < 0) num = 0;
	SetAnimationSpeed (num);
	return 1;

      case kwn_BorderShadowDepth:
	if (Scr->FirstTime) Scr->BorderShadowDepth = num;
	if (Scr->BorderShadowDepth < 0) Scr->BorderShadowDepth = 2;
	return 1;

      case kwn_BorderLeft:
	if (Scr->FirstTime) Scr->BorderLeft = num;
	if (Scr->BorderLeft < 0) Scr->BorderLeft = 0;
	return 1;

      case kwn_BorderRight:
	if (Scr->FirstTime) Scr->BorderRight = num;
	if (Scr->BorderRight < 0) Scr->BorderRight = 0;
	return 1;

      case kwn_BorderTop:
	if (Scr->FirstTime) Scr->BorderTop = num;
	if (Scr->BorderTop < 0) Scr->BorderTop = 0;
	return 1;

      case kwn_BorderBottom:
	if (Scr->FirstTime) Scr->BorderBottom = num;
	if (Scr->BorderBottom < 0) Scr->BorderBottom = 0;
	return 1;

      case kwn_TitleButtonShadowDepth:
	if (Scr->FirstTime) Scr->TitleButtonShadowDepth = num;
	if (Scr->TitleButtonShadowDepth < 0) Scr->TitleButtonShadowDepth = 2;
	return 1;

      case kwn_TitleShadowDepth:
	if (Scr->FirstTime) Scr->TitleShadowDepth = num;
	if (Scr->TitleShadowDepth < 0) Scr->TitleShadowDepth = 2;
	return 1;

      case kwn_IconManagerShadowDepth:
	if (Scr->FirstTime) Scr->IconManagerShadowDepth = num;
	if (Scr->IconManagerShadowDepth < 0) Scr->IconManagerShadowDepth = 2;
	return 1;

      case kwn_MenuShadowDepth:
	if (Scr->FirstTime) Scr->MenuShadowDepth = num;
	if (Scr->MenuShadowDepth < 0) Scr->MenuShadowDepth = 2;
	return 1;

      case kwn_OpenWindowTimeout:
	if (Scr->FirstTime) Scr->OpenWindowTimeout = num;
	if (Scr->OpenWindowTimeout < 0) Scr->OpenWindowTimeout = 0;
	return 1;

      case kwn_RaiseOnClickButton:
	if (Scr->FirstTime) Scr->RaiseOnClickButton = num;
	if (Scr->RaiseOnClickButton < 1) Scr->RaiseOnClickButton = 1;
	if (Scr->RaiseOnClickButton > MAX_BUTTONS) Scr->RaiseOnClickButton = MAX_BUTTONS;
	return 1;

    }

    return 0;
}

name_list **do_colorlist_keyword (int keyword, int colormode, char *s)
{
    switch (keyword) {
      case kwcl_BorderColor:
	GetColor (colormode, &Scr->BorderColorC.back, s);
	return &Scr->BorderColorL;

      case kwcl_IconManagerHighlight:
	GetColor (colormode, &Scr->IconManagerHighlight, s);
	return &Scr->IconManagerHighlightL;

      case kwcl_BorderTileForeground:
	GetColor (colormode, &Scr->BorderTileC.fore, s);
	return &Scr->BorderTileForegroundL;

      case kwcl_BorderTileBackground:
	GetColor (colormode, &Scr->BorderTileC.back, s);
	return &Scr->BorderTileBackgroundL;

      case kwcl_TitleForeground:
	GetColor (colormode, &Scr->TitleC.fore, s);
	return &Scr->TitleForegroundL;

      case kwcl_TitleBackground:
	GetColor (colormode, &Scr->TitleC.back, s);
	return &Scr->TitleBackgroundL;

      case kwcl_IconForeground:
	GetColor (colormode, &Scr->IconC.fore, s);
	return &Scr->IconForegroundL;

      case kwcl_IconBackground:
	GetColor (colormode, &Scr->IconC.back, s);
	return &Scr->IconBackgroundL;

      case kwcl_IconBorderColor:
	GetColor (colormode, &Scr->IconBorderColor, s);
	return &Scr->IconBorderColorL;

      case kwcl_IconManagerForeground:
	GetColor (colormode, &Scr->IconManagerC.fore, s);
	return &Scr->IconManagerFL;

      case kwcl_IconManagerBackground:
	GetColor (colormode, &Scr->IconManagerC.back, s);
	return &Scr->IconManagerBL;

      case kwcl_MapWindowBackground:
	GetColor (colormode, &Scr->workSpaceMgr.windowcp.back, s);
	Scr->workSpaceMgr.windowcpgiven = True;
	return &Scr->workSpaceMgr.windowBackgroundL;

      case kwcl_MapWindowForeground:
	GetColor (colormode, &Scr->workSpaceMgr.windowcp.fore, s);
	Scr->workSpaceMgr.windowcpgiven = True;
	return &Scr->workSpaceMgr.windowForegroundL;
    }
    return NULL;
}

int do_color_keyword (int keyword, int colormode, char *s)
{
    switch (keyword) {
      case kwc_DefaultForeground:
	GetColor (colormode, &Scr->DefaultC.fore, s);
	return 1;

      case kwc_DefaultBackground:
	GetColor (colormode, &Scr->DefaultC.back, s);
	return 1;

      case kwc_MenuForeground:
	GetColor (colormode, &Scr->MenuC.fore, s);
	return 1;

      case kwc_MenuBackground:
	GetColor (colormode, &Scr->MenuC.back, s);
	return 1;

      case kwc_MenuTitleForeground:
	GetColor (colormode, &Scr->MenuTitleC.fore, s);
	return 1;

      case kwc_MenuTitleBackground:
	GetColor (colormode, &Scr->MenuTitleC.back, s);
	return 1;

      case kwc_MenuShadowColor:
	GetColor (colormode, &Scr->MenuShadowColor, s);
	return 1;

    }

    return 0;
}

/*
 * put_pixel_on_root() Save a pixel value in twm root window color property.
 */
static void put_pixel_on_root(Pixel pixel)
{
  int           i, addPixel = 1;
  Atom          retAtom;
  int           retFormat;
  unsigned long nPixels, retAfter;
  Pixel        *retProp;

  if (XGetWindowProperty(dpy, Scr->Root, _XA_MIT_PRIORITY_COLORS, 0, 8192,
		     False, XA_CARDINAL, &retAtom,
		     &retFormat, &nPixels, &retAfter,
		     (unsigned char **)&retProp) != Success || !retProp)
      return;

  for (i=0; i< nPixels; i++)
      if (pixel == retProp[i]) addPixel = 0;

  XFree((char *)retProp);

  if (addPixel)
      XChangeProperty (dpy, Scr->Root, _XA_MIT_PRIORITY_COLORS,
		       XA_CARDINAL, 32, PropModeAppend,
		       (unsigned char *)&pixel, 1);
}

/*
 * do_string_savecolor() save a color from a string in the twmrc file.
 */
int do_string_savecolor(int colormode, char *s)
{
  Pixel p;
  GetColor(colormode, &p, s);
  put_pixel_on_root(p);
  return 0;
}

/*
 * do_var_savecolor() save a color from a var in the twmrc file.
 */
typedef struct _cnode {int i; struct _cnode *next;} Cnode, *Cptr;
Cptr chead = NULL;

int do_var_savecolor(int key)
{
  Cptr cptrav, cpnew;
  if (!chead) {
    chead = (Cptr)malloc(sizeof(Cnode));
    chead->i = key; chead->next = NULL;
  }
  else {
    cptrav = chead;
    while (cptrav->next != NULL) { cptrav = cptrav->next; }
    cpnew = (Cptr)malloc(sizeof(Cnode));
    cpnew->i = key; cpnew->next = NULL; cptrav->next = cpnew;
  }
  return 0;
}

/*
 * assign_var_savecolor() traverse the var save color list placeing the pixels
 *                        in the root window property.
 */
void assign_var_savecolor(void)
{
  Cptr cp = chead;
  while (cp != NULL) {
    switch (cp->i) {
    case kwcl_BorderColor:
      put_pixel_on_root(Scr->BorderColorC.back);
      break;
    case kwcl_IconManagerHighlight:
      put_pixel_on_root(Scr->IconManagerHighlight);
      break;
    case kwcl_BorderTileForeground:
      put_pixel_on_root(Scr->BorderTileC.fore);
      break;
    case kwcl_BorderTileBackground:
      put_pixel_on_root(Scr->BorderTileC.back);
      break;
    case kwcl_TitleForeground:
      put_pixel_on_root(Scr->TitleC.fore);
      break;
    case kwcl_TitleBackground:
      put_pixel_on_root(Scr->TitleC.back);
      break;
    case kwcl_IconForeground:
      put_pixel_on_root(Scr->IconC.fore);
      break;
    case kwcl_IconBackground:
      put_pixel_on_root(Scr->IconC.back);
      break;
    case kwcl_IconBorderColor:
      put_pixel_on_root(Scr->IconBorderColor);
      break;
    case kwcl_IconManagerForeground:
      put_pixel_on_root(Scr->IconManagerC.fore);
      break;
    case kwcl_IconManagerBackground:
      put_pixel_on_root(Scr->IconManagerC.back);
      break;
    case kwcl_MapWindowForeground:
      put_pixel_on_root(Scr->workSpaceMgr.windowcp.fore);
      break;
    case kwcl_MapWindowBackground:
      put_pixel_on_root(Scr->workSpaceMgr.windowcp.back);
      break;
    }
    cp = cp->next;
  }
  if (chead) {
    free(chead);
    chead = NULL;
  }
}

static int ParseRandomPlacement (register char *s)
{
    if (strlen (s) == 0) return RP_ALL;
    if (strcmp (s, "default") == 0) return RP_ALL;
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,      "off") == 0) return RP_OFF;
    if (strcmp (s,       "on") == 0) return RP_ALL;
    if (strcmp (s,      "all") == 0) return RP_ALL;
    if (strcmp (s, "unmapped") == 0) return RP_UNMAPPED;
    return (-1);
}

int ParseJustification (register char *s)
{
    if (strlen (s) == 0) return (-1);
    if (strcmp (s, "default") == 0) return J_CENTER;
    if (strcmp (s,   "undef") == 0) return J_UNDEF;
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,   "undef") == 0) return J_UNDEF;
    if (strcmp (s, "default") == 0) return J_CENTER;
    if (strcmp (s,    "left") == 0) return J_LEFT;
    if (strcmp (s,  "center") == 0) return J_CENTER;
    if (strcmp (s,   "right") == 0) return J_RIGHT;
    if (strcmp (s,  "border") == 0) return J_BORDER;
    return (-1);
}

int ParseAlignement (register char *s)
{
    if (strlen (s) == 0) return (-1);
    if (strcmp (s, "default") == 0) return J_CENTER;
    if (strcmp (s,   "undef") == 0) return J_UNDEF;
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,   "undef") == 0) return J_UNDEF;
    if (strcmp (s, "default") == 0) return J_CENTER;
    if (strcmp (s,     "top") == 0) return J_TOP;
    if (strcmp (s,  "center") == 0) return J_CENTER;
    if (strcmp (s,  "bottom") == 0) return J_BOTTOM;
    if (strcmp (s,  "border") == 0) return J_BORDER;
    return (-1);
}

static int ParseUsePPosition (register char *s)
{
    if (strlen (s) == 0) return (-1);
    if (strcmp (s,  "default") == 0) return PPOS_OFF;
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,  "default") == 0) return PPOS_OFF;
    if (strcmp (s,      "off") == 0) return PPOS_OFF;
    if (strcmp (s,       "on") == 0) return PPOS_ON;
    if (strcmp (s, "non-zero") == 0) return PPOS_NON_ZERO;
    if (strcmp (s,  "nonzero") == 0) return PPOS_NON_ZERO;
    return (-1);
}

static int ParseButtonStyle (register char *s)
{
    if (strlen (s) == 0) return (-1);
    if (strcmp (s,  "default") == 0) return STYLE_NORMAL;
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s, "normal") == 0) return STYLE_NORMAL;
    if (strcmp (s, "style1") == 0) return STYLE_STYLE1;
    if (strcmp (s, "style2") == 0) return STYLE_STYLE2;
    if (strcmp (s, "style3") == 0) return STYLE_STYLE3;
    return (-1);
}

int do_squeeze_entry (name_list **list,	/* squeeze or dont-squeeze list */
		      char *name,	/* window name */
		      int justify,	/* left, center, right */
		      int num,		/* signed num */
		      int denom)	/* 0 or indicates fraction denom */
{
    int absnum = (num < 0 ? -num : num);

    if (denom < 0) {
	twmrc_error_prefix();
	fprintf (stderr, "negative SqueezeTitle denominator %d\n", denom);
	return (1);
    }
    if (absnum > denom && denom != 0) {
	twmrc_error_prefix();
	fprintf (stderr, "SqueezeTitle fraction %d/%d outside window\n",
		 num, denom);
	return (1);
    }
    /* Process the special cases from the manual here rather than
     * each time we calculate the position of the title bar
     * in add_window.c:ComputeTitleLocation().
     * In fact, it's better to get rid of them entirely, but we
     * probably should not do that for compatibility's sake.
     * By using a non-zero denominator the position will be relative.
     */
    if (denom == 0 && num == 0) {
	if (justify == J_CENTER) {
	    num = 1;
	    denom = 2;
	} else if (justify == J_RIGHT) {
	    num = 2;
	    denom = 2;
	}
	twmrc_error_prefix();
	fprintf (stderr, "deprecated SqueezeTitle faction 0/0, assuming %d/%d\n",
		 num, denom);
    }

    if (HasShape) {
	SqueezeInfo *sinfo;
	sinfo = (SqueezeInfo *) malloc (sizeof(SqueezeInfo));

	if (!sinfo) {
	    twmrc_error_prefix();
	    fprintf (stderr, "unable to allocate %lu bytes for squeeze info\n",
		     (unsigned long) sizeof(SqueezeInfo));
	    return (1);
	}
	sinfo->justify = justify;
	sinfo->num = num;
	sinfo->denom = denom;
	AddToList (list, name, (char *) sinfo);
    }
    return (0);
}

#ifdef USEM4

static FILE *start_m4(FILE *fraw)
{
        int fno;
        int fids[2];
        int fres;               /* Fork result */

        fno = fileno(fraw);
        /* if (-1 == fcntl(fno, F_SETFD, 0)) perror("fcntl()"); */
        if (pipe(fids) < 0) {
                perror("Pipe for " M4CMD " failed");
                exit(23);
	}
        fres = fork();
        if (fres < 0) {
                perror("Fork for " M4CMD " failed");
                exit(23);
        }
        if (fres == 0) {
                char *tmp_file;

                /* Child */
                close(0);                       /* stdin */
                close(1);                       /* stdout */
                dup2(fno, 0);           /* stdin = fraw */
                dup2(fids[1], 1);       /* stdout = pipe to parent */
                /* get_defs("m4", dpy, display_name) */
                tmp_file = m4_defs(dpy, display_name);
                execlp(M4CMD, M4CMD, "-s", tmp_file, "-", NULL);

                /* If we get here we are screwed... */
                perror("Can't execlp() " M4CMD);
                exit(124);
        }
        /* Parent */
        close(fids[1]);
        return ((FILE*)fdopen(fids[0], "r"));
}

/* Code taken and munged from xrdb.c */
#define MAXHOSTNAME 255
#define Resolution(pixels, mm)  ((((pixels) * 100000 / (mm)) + 50) / 100)
#define EXTRA   16

static char *MkDef(char *name, char *def)
{
        static char *cp = NULL;
        static int maxsize = 0;
        int n, nl;

	if (def == NULL) return ("");     /* XXX JWS: prevent segfaults */
        /* The char * storage only lasts for 1 call... */
        if ((n = EXTRA + ((nl = strlen(name)) +  strlen(def))) > maxsize) {
		maxsize = n;
                if (cp) free(cp);

                cp = malloc(n);
        }
        /* Otherwise cp is aready big 'nuf */
        if (cp == NULL) {
                fprintf(stderr, "Can't get %d bytes for arg parm\n", n);
                exit(468);
        }
	strcpy (cp, "define(`");
	strcat (cp, name);
	strcat (cp, "', `");
	strcat (cp, def);
	strcat (cp, "')\n");

        return(cp);
}

static char *MkNum(char *name, int def)
{
        char num[20];

        sprintf(num, "%d", def);
        return(MkDef(name, num));
}

#ifdef NOSTEMP
int mkstemp(str)
char *str;
{
        int fd;

        mktemp(str);
        fd = creat(str, 0744);
        if (fd == -1) perror("mkstemp's creat");
        return(fd);
}
#endif

static char *m4_defs(Display *display, char *host)
{
        Screen *screen;
        Visual *visual;
        char client[MAXHOSTNAME], server[MAXHOSTNAME], *colon;
        struct hostent *hostname;
        char *vc;               /* Visual Class */
        static char tmp_name[] = "/tmp/twmrcXXXXXX";
        int fd;
        FILE *tmpf;
	char *user;

        fd = mkstemp(tmp_name);		/* I *hope* mkstemp exists, because */
					/* I tried to find the "portable" */
					/* mktmp... */
        if (fd < 0) {
                perror("mkstemp failed in m4_defs");
                exit(377);
        }
        tmpf = (FILE*) fdopen(fd, "w+");
        XmuGetHostname(client, MAXHOSTNAME);
        hostname = gethostbyname(client);
        strcpy(server, XDisplayName(host));
        colon = strchr(server, ':');
        if (colon != NULL) *colon = '\0';
        if ((server[0] == '\0') || (!strcmp(server, "unix")))
                strcpy(server, client); /* must be connected to :0 or unix:0 */
        /* The machine running the X server */
        fputs(MkDef("SERVERHOST", server), tmpf);
        /* The machine running the window manager process */
        fputs(MkDef("CLIENTHOST", client), tmpf);
        if (hostname)
                fputs(MkDef("HOSTNAME", hostname->h_name), tmpf);
        else
                fputs(MkDef("HOSTNAME", client), tmpf);

	if (!(user=getenv("USER")) && !(user=getenv("LOGNAME"))) user = "unknown";
        fputs(MkDef("USER", user), tmpf);
        fputs(MkDef("HOME", getenv("HOME")), tmpf);
        fputs(MkDef("PIXMAP_DIRECTORY", PIXMAP_DIRECTORY), tmpf);
        fputs(MkNum("VERSION", ProtocolVersion(display)), tmpf);
        fputs(MkNum("REVISION", ProtocolRevision(display)), tmpf);
        fputs(MkDef("VENDOR", ServerVendor(display)), tmpf);
        fputs(MkNum("RELEASE", VendorRelease(display)), tmpf);
        screen = ScreenOfDisplay(display, Scr->screen);
        visual = DefaultVisualOfScreen(screen);
        fputs(MkNum("WIDTH", screen->width), tmpf);
        fputs(MkNum("HEIGHT", screen->height), tmpf);
        fputs(MkNum("X_RESOLUTION",Resolution(screen->width,screen->mwidth)), tmpf);
        fputs(MkNum("Y_RESOLUTION",Resolution(screen->height,screen->mheight)),tmpf);
        fputs(MkNum("PLANES",DisplayPlanes(display, Scr->screen)), tmpf);
        fputs(MkNum("BITS_PER_RGB", visual->bits_per_rgb), tmpf);
        fputs(MkDef("TWM_TYPE", "ctwm"), tmpf);
        fputs(MkDef("TWM_VERSION", VersionNumber), tmpf);
        switch(visual->class) {
                case(StaticGray):       vc = "StaticGray";      break;
                case(GrayScale):        vc = "GrayScale";       break;
                case(StaticColor):      vc = "StaticColor";     break;
                case(PseudoColor):      vc = "PseudoColor";     break;
                case(TrueColor):        vc = "TrueColor";       break;
                case(DirectColor):      vc = "DirectColor";     break;
                default:                vc = "NonStandard";     break;
        }
        fputs(MkDef("CLASS", vc), tmpf);
        if (visual->class != StaticGray && visual->class != GrayScale) {
                fputs(MkDef("COLOR", "Yes"), tmpf);
        } else {
                fputs(MkDef("COLOR", "No"), tmpf);
        }
#ifdef XPM
	fputs(MkDef("XPM", "Yes"), tmpf);
#endif
#ifdef JPEG
	fputs(MkDef("JPEG", "Yes"), tmpf);
#endif
#ifdef IMCONV
	fputs(MkDef("IMCONV", "Yes"), tmpf);
#endif
#ifdef GNOME
	fputs(MkDef("GNOME", "Yes"), tmpf);
#endif
#ifdef SOUNDS
	fputs(MkDef("SOUNDS", "Yes"), tmpf);
#endif
	fputs(MkDef("I18N", "Yes"), tmpf);
	if (captive && captivename) {
            fputs (MkDef ("TWM_CAPTIVE", "Yes"), tmpf);
            fputs (MkDef ("TWM_CAPTIVE_NAME", captivename), tmpf);
	} else {
            fputs (MkDef ("TWM_CAPTIVE", "No"), tmpf);
	}
        if (KeepTmpFile) {
                fprintf(stderr, "Left file: %s\n", tmp_name);
        } else {
                fprintf(tmpf, "syscmd(/bin/rm %s)\n", tmp_name);
        }
        fclose(tmpf);
        return(tmp_name);
}
#endif /* USEM4 */
