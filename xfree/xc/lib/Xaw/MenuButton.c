/* $TOG: MenuButton.c /main/24 1998/06/24 11:53:20 kaleb $ */

/*
Copyright 1989, 1994, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 */

/* $XFree86: xc/lib/Xaw/MenuButton.c,v 3.7 1999/06/06 08:47:59 dawes Exp $ */

/*
 * MenuButton.c - Source code for MenuButton widget.
 *
 * This is the source code for the Athena MenuButton widget.
 * It is intended to provide an easy method of activating pulldown menus.
 *
 * Date:    May 2, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/SysUtil.h>
#include <X11/Xaw/MenuButtoP.h>
#include <X11/Xaw/XawInit.h>
#include "Private.h"

/*
 * Class Methods
 */
static void XawMenuButtonClassInitialize(void);

/*
 * Actions
 */
static void PopupMenu(Widget, XEvent*, String*, Cardinal*);

/*
 * Initialization
 */
#define superclass ((CommandWidgetClass)&commandClassRec)

static char defaultTranslations[] = 
"<Enter>:"	"highlight()\n"
"<Leave>:"	"reset()\n"
"Any<BtnDown>:"	"reset() PopupMenu()\n";

#define offset(field) XtOffsetOf(MenuButtonRec, field)
static XtResource resources[] = {
  {
    XtNmenuName,
    XtCMenuName,
    XtRString,
    sizeof(String),
    offset(menu_button.menu_name),
    XtRString,
    (XtPointer)"menu"
  },
};
#undef offset

static XtActionsRec actionsList[] =
{
  {"PopupMenu",	PopupMenu},
};

MenuButtonClassRec menuButtonClassRec = {
  /* core */
  {
    (WidgetClass)superclass,		/* superclass		  */
    "MenuButton",			/* class_name		  */
    sizeof(MenuButtonRec),       	/* size			  */
    XawMenuButtonClassInitialize,	/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    False,				/* class_inited		  */
    NULL,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* num_resources	  */
    NULLQUARK,				/* xrm_class		  */
    False,				/* compress_motion	  */
    True,				/* compress_exposure	  */
    True,				/* compress_enterleave	  */
    False,				/* visible_interest	  */
    NULL,				/* destroy		  */
    XtInheritResize,			/* resize		  */
    XtInheritExpose,			/* expose		  */
    NULL,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,               	/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive		/* change_sensitive	  */ 
  },
  /* label */
  {
    NULL,				/* extension */
  },
  /* command */
  {
    NULL,				/* extension */
  },
  /* menu_button */
  {
    NULL,				/* extension */
  },
};

WidgetClass menuButtonWidgetClass = (WidgetClass)&menuButtonClassRec;

/*
 * Implementation
 */
static void
XawMenuButtonClassInitialize(void)
{
    XawInitializeWidgetSet();
    XtRegisterGrabAction(PopupMenu, True,
			 ButtonPressMask | ButtonReleaseMask,
			 GrabModeAsync, GrabModeAsync);
}

/*ARGSUSED*/
static void
PopupMenu(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    MenuButtonWidget mbw = (MenuButtonWidget)w;
    Widget menu = NULL, temp;
    Arg arglist[2];
    Cardinal num_args;
    int menu_x, menu_y, menu_width, menu_height, button_height;
    Position button_x, button_y;

    temp = w;
    while(temp != NULL) {
	menu = XtNameToWidget(temp, mbw->menu_button.menu_name);
	if (menu == NULL) 
	    temp = XtParent(temp);
	else
	    break;
    }

    if (menu == NULL) {
	char error_buf[BUFSIZ];

	(void)XmuSnprintf(error_buf, sizeof(error_buf),
			  "MenuButton:  Could not find menu widget named %s.",
			  mbw->menu_button.menu_name);
	XtAppWarning(XtWidgetToApplicationContext(w), error_buf);
	return;
    }

    if (!XtIsRealized(menu))
	XtRealizeWidget(menu);
  
    menu_width = XtWidth(menu) + (XtBorderWidth(menu) << 1);
    button_height = XtHeight(w) + (XtBorderWidth(w) << 1);
    menu_height = XtHeight(menu) + (XtBorderWidth(menu) << 1);

    XtTranslateCoords(w, 0, 0, &button_x, &button_y);
    menu_x = button_x;
    menu_y = button_y + button_height;

    if (menu_y >= 0) {
	int scr_height = HeightOfScreen(XtScreen(menu));

	if (menu_y + menu_height > scr_height)
	    menu_y = button_y - menu_height;
	if (menu_y < 0) {
	    menu_y = scr_height - menu_height;
	    menu_x = button_x + XtWidth(w) + (XtBorderWidth(w) << 1);
	    if (menu_x + menu_width > WidthOfScreen(XtScreen(menu)))
		menu_x = button_x - menu_width;
	}
    }
    if (menu_y < 0)
	menu_y = 0;

    if (menu_x >= 0) {
	int scr_width = WidthOfScreen(XtScreen(menu));

	if (menu_x + menu_width > scr_width)
	    menu_x = scr_width - menu_width;
    }
    if (menu_x < 0) 
	menu_x = 0;

    num_args = 0;
    XtSetArg(arglist[num_args], XtNx, menu_x); num_args++;
    XtSetArg(arglist[num_args], XtNy, menu_y); num_args++;
    XtSetValues(menu, arglist, num_args);

    XtPopupSpringLoaded(menu);
}
