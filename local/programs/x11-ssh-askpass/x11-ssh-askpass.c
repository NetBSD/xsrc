/* x11-ssh-askpass.c:  A generic X11-based password dialog for OpenSSH.
 * created 1999-Nov-17 03:40 Jim Knoble <jmknoble@pobox.com>
 * autodate: 1999-Nov-23 02:52
 * 
 * by Jim Knoble <jmknoble@pobox.com>
 * Copyright © 1999 Jim Knoble
 * 
 * Disclaimer:
 * 
 * The software is provided "as is", without warranty of any kind,
 * express or implied, including but not limited to the warranties of
 * merchantability, fitness for a particular purpose and
 * noninfringement. In no event shall the author(s) be liable for any
 * claim, damages or other liability, whether in an action of
 * contract, tort or otherwise, arising from, out of or in connection
 * with the software or the use or other dealings in the software.
 * 
 * Portions of this code are distantly derived from code in xscreensaver
 * by Jamie Zawinski <jwz@jwz.org>.  That code says:
 * 
 * --------8<------------------------------------------------8<--------
 * xscreensaver, Copyright (c) 1991-1999 Jamie Zawinski <jwz@jwz.org>
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 * --------8<------------------------------------------------8<--------
 * 
 * The remainder of this code falls under the same permissions and
 * provisions as those of the xscreensaver code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For (get|set)rlimit() ... */
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
/* ... end */

/* For errno ... */
#include <errno.h>
/* ... end */

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xos.h>
#include "dynlist.h"
#include "drawing.h"
#include "resources.h"
#include "x11-ssh-askpass.h"

#undef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

char *progname = NULL;
char *progclass = NULL;
XrmDatabase db = 0;

static char *defaults[] = {
#include "SshAskpass_ad.h"
   0
};

void outOfMemory(AppInfo *app, int line)
{
   fprintf(stderr, "%s: Aaahhh! I ran out of memory at line %d.\n",
	   app->appName, line);
   exit(EXIT_STATUS_NO_MEMORY);
}

void freeIf(void *p)
{
   if (p) {
      free(p);
   }
}

void freeFontIf(AppInfo *app, XFontStruct *f)
{
   if (f) {
      XFreeFont(app->dpy, f);
   }
}

XFontStruct *getFontResource(AppInfo *app, char *instanceName, char *className)
{
   char *fallbackFont = "fixed";
   
   XFontStruct *f = NULL;
   char *s = get_string_resource(instanceName, className);
   f = XLoadQueryFont(app->dpy, (s ? s : fallbackFont));
   if (!f) {
      f = XLoadQueryFont(app->dpy, fallbackFont);
   }
   if (s) {
      free(s);
   }
   return(f);
}

char *getStringResourceWithDefault(char *instanceName, char *className,
				   char *defaultText)
{
   char *s = get_string_resource(instanceName, className);
   if (!s) {
      if (!defaultText) {
	 s = strdup("");
      } else {
	 s = strdup(defaultText);
      }
   }
   return(s);
}

void calcLabelTextExtents(LabelInfo *label)
{
   if ((!label) || (!(label->text)) || (!(label->font))) {
      return;
   }
   label->textLength = strlen(label->text);
   XTextExtents(label->font, label->text, label->textLength,
		&(label->direction), &(label->ascent), &(label->descent),
		&(label->overall));
   label->w.height = label->descent + label->ascent;
   label->w.width = label->overall.width;
}

void calcTotalButtonExtents(ButtonInfo *button)
{
   if (!button) {
      return;
   }
   button->w3.w.width = (button->w3.interiorWidth + 
			 (2 * button->w3.shadowThickness));
   button->w3.w.width += (2 * button->w3.borderWidth);
   button->w3.w.height = (button->w3.interiorHeight +
			  (2 * button->w3.shadowThickness));
   button->w3.w.height += (2 * button->w3.borderWidth);
}

void calcButtonExtents(ButtonInfo *button)
{
   if (!button) {
      return;
   }
   calcLabelTextExtents(&(button->label));
   button->w3.interiorWidth = (button->label.w.width +
			       (2 * button->w3.horizontalSpacing));
   button->w3.interiorHeight = (button->label.w.height +
				(2 * button->w3.verticalSpacing));
   calcTotalButtonExtents(button);
}

void balanceButtonExtents(ButtonInfo *button1, ButtonInfo *button2)
{
   if ((!button1) || (!button2)) {
      return;
   }
   button1->w3.interiorWidth = button2->w3.interiorWidth = 
      MAX(button1->w3.interiorWidth, button2->w3.interiorWidth);
   button1->w3.interiorHeight = button2->w3.interiorHeight =
      MAX(button1->w3.interiorHeight, button2->w3.interiorHeight);
   calcTotalButtonExtents(button1);
   calcTotalButtonExtents(button2);
}

void calcButtonLabelPosition(ButtonInfo *button)
{
   if (!button) {
      return;
   }
   button->label.w.x = button->w3.w.x +
      ((button->w3.w.width - button->label.w.width) / 2);
   button->label.w.y = button->w3.w.y +
      ((button->w3.w.height - button->label.w.height) / 2)
      + button->label.ascent;
}

void createDialog(AppInfo *app)
{
   DialogInfo *d;
   
   if (app->dialog) {
      return;
   }
   d = malloc(sizeof(*d));
   if (NULL == d) {
      outOfMemory(app, __LINE__);
   }
   memset(d, 0, sizeof(*d));

   app->grabKeyboard =
      get_boolean_resource("grabKeyboard", "GrabKeyboard", True);
   app->grabPointer =
      get_boolean_resource("grabPointer", "GrabPointer", False);
   app->grabPointer =
      get_boolean_resource("grabServer", "GrabServer", False);
   
   d->title =
      getStringResourceWithDefault("dialog.title", "Dialog.Title",
				   "OpenSSH Authentication Passphrase Request");
   d->w3.w.foreground =
      get_pixel_resource("foreground", "Foreground",
			 app->dpy, app->colormap, app->black);
   d->w3.w.background =
      get_pixel_resource("background", "Background",
			 app->dpy, app->colormap, app->white);
   d->w3.topShadowColor =
      get_pixel_resource("topShadowColor", "TopShadowColor",
			 app->dpy, app->colormap, app->white);
   d->w3.bottomShadowColor =
      get_pixel_resource("bottomShadowColor", "BottomShadowColor",
			 app->dpy, app->colormap, app->black);
   d->w3.shadowThickness =
      get_integer_resource("shadowThickness", "ShadowThickness", 3);
   d->w3.borderColor =
      get_pixel_resource("borderColor", "BorderColor",
			 app->dpy, app->colormap, app->black);
   d->w3.borderWidth =
      get_integer_resource("borderWidth", "BorderWidth", 1);
   
   d->w3.horizontalSpacing =
      get_integer_resource("horizontalSpacing", "Spacing", 5);
   d->w3.verticalSpacing =
      get_integer_resource("verticalSpacing", "Spacing", 6);
   
   if (2 == app->argc) {
      d->label.text = strdup(app->argv[1]);
   } else {
      d->label.text =
	 getStringResourceWithDefault("dialog.label", "Dialog.Label",
				      "Please enter your authentication passphrase:");
   }
   d->label.font = getFontResource(app, "dialog.font", "Dialog.Font");
   calcLabelTextExtents(&(d->label));
   d->label.w.foreground = d->w3.w.foreground;
   d->label.w.background = d->w3.w.background;
   
   d->okButton.w3.w.foreground =
      get_pixel_resource("okButton.foreground", "Button.Foreground",
			 app->dpy, app->colormap, app->black);
   d->okButton.w3.w.background =
      get_pixel_resource("okButton.background", "Button.Background",
			 app->dpy, app->colormap, app->white);
   d->okButton.w3.topShadowColor =
      get_pixel_resource("okButton.topShadowColor", "Button.TopShadowColor",
			 app->dpy, app->colormap, app->white);
   d->okButton.w3.bottomShadowColor =
      get_pixel_resource("okButton.bottomShadowColor",
			 "Button.BottomShadowColor",
			 app->dpy, app->colormap, app->black);
   d->okButton.w3.shadowThickness =
      get_integer_resource("okButton.shadowThickness",
			   "Button.ShadowThickness", 2);
   d->okButton.w3.borderColor =
      get_pixel_resource("okButton.borderColor", "Button.BorderColor",
			 app->dpy, app->colormap, app->black);
   d->okButton.w3.borderWidth =
      get_integer_resource("okButton.borderWidth", "Button.BorderWidth", 1);
   d->okButton.w3.horizontalSpacing = 
      get_integer_resource("okButton.horizontalSpacing", "Button.Spacing", 4);
   d->okButton.w3.verticalSpacing = 
      get_integer_resource("okButton.verticalSpacing", "Button.Spacing", 2);
   d->okButton.label.text =
      getStringResourceWithDefault("okButton.label", "Button.Label", "OK");
   d->okButton.label.font =
      getFontResource(app, "okButton.font", "Button.Font");
   calcButtonExtents(&(d->okButton));
   d->okButton.label.w.foreground = d->okButton.w3.w.foreground;
   d->okButton.label.w.background = d->okButton.w3.w.background;
   
   d->cancelButton.w3.w.foreground =
      get_pixel_resource("cancelButton.foreground", "Button.Foreground",
			 app->dpy, app->colormap, app->black);
   d->cancelButton.w3.w.background =
      get_pixel_resource("cancelButton.background", "Button.Background",
			 app->dpy, app->colormap, app->white);
   d->cancelButton.w3.topShadowColor =
      get_pixel_resource("cancelButton.topShadowColor",
			 "Button.TopShadowColor",
			 app->dpy, app->colormap, app->white);
   d->cancelButton.w3.bottomShadowColor =
      get_pixel_resource("cancelButton.bottomShadowColor",
			 "Button.BottomShadowColor",
			 app->dpy, app->colormap, app->black);
   d->cancelButton.w3.shadowThickness =
      get_integer_resource("cancelButton.shadowThickness",
			   "Button.ShadowThickness", 2);
   d->cancelButton.w3.borderColor =
      get_pixel_resource("cancelButton.borderColor", "Button.BorderColor",
			 app->dpy, app->colormap, app->black);
   d->cancelButton.w3.borderWidth =
      get_integer_resource("cancelButton.borderWidth", "Button.BorderWidth",
			   1);
   d->cancelButton.w3.horizontalSpacing = 
      get_integer_resource("cancelButton.horizontalSpacing", "Button.Spacing",
			   4);
   d->cancelButton.w3.verticalSpacing = 
      get_integer_resource("cancelButton.verticalSpacing", "Button.Spacing",
			   2);
   d->cancelButton.label.text =
      getStringResourceWithDefault("cancelButton.label", "Button.Label",
				   "Cancel");
   d->cancelButton.label.font =
      getFontResource(app, "cancelButton.font", "Button.Font");
   calcButtonExtents(&(d->cancelButton));
   d->cancelButton.label.w.foreground = d->cancelButton.w3.w.foreground;
   d->cancelButton.label.w.background = d->cancelButton.w3.w.background;

   balanceButtonExtents(&(d->okButton), &(d->cancelButton));
   
   d->indicator.w3.w.foreground =
      get_pixel_resource("indicator.foreground", "Indicator.Foreground",
			 app->dpy, app->colormap, app->black);
   d->indicator.w3.w.background =
      get_pixel_resource("indicator.background", "Indicator.Background",
			 app->dpy, app->colormap, app->white);
   d->indicator.w3.w.width =
      get_integer_resource("indicator.width", "Indicator.Width", 15);
   d->indicator.w3.w.height =
      get_integer_resource("indicator.height", "Indicator.Height", 7);
   d->indicator.w3.topShadowColor =
      get_pixel_resource("indicator.topShadowColor",
			 "Indicator.TopShadowColor",
			 app->dpy, app->colormap, app->white);
   d->indicator.w3.bottomShadowColor =
      get_pixel_resource("indicator.bottomShadowColor",
			 "Indicator.BottomShadowColor",
			 app->dpy, app->colormap, app->black);
   d->indicator.w3.shadowThickness =
      get_integer_resource("indicator.shadowThickness",
			   "Indicator.ShadowThickness", 2);
   d->indicator.w3.borderColor =
      get_pixel_resource("indicator.borderColor", "Indicator.BorderColor",
			 app->dpy, app->colormap, app->black);
   d->indicator.w3.borderWidth =
      get_integer_resource("indicator.borderWidth", "Indicator.BorderWidth",
			   0);
   d->indicator.w3.horizontalSpacing =
      get_integer_resource("indicator.horizontalSpacing", "Indicator.Spacing",
			   2);
   d->indicator.w3.verticalSpacing =
      get_integer_resource("indicator.verticalSpacing", "Indicator.Spacing",
			   4);
   d->indicator.minimumCount =
      get_integer_resource("indicator.minimumCount", "Indicator.MinimumCount",
			   8);
   d->indicator.maximumCount =
      get_integer_resource("indicator.maximumCount", "Indicator.MaximumCount",
			   24);
   d->indicator.w3.interiorWidth = d->indicator.w3.w.width;
   d->indicator.w3.interiorHeight = d->indicator.w3.w.height;
   d->indicator.w3.w.width += (2 * d->indicator.w3.shadowThickness);
   d->indicator.w3.w.width += (2 * d->indicator.w3.borderWidth);
   d->indicator.w3.w.height += (2 * d->indicator.w3.shadowThickness);
   d->indicator.w3.w.height += (2 * d->indicator.w3.borderWidth);
   {
      /* Make sure the indicators can all fit on the screen.
       * 80% of the screen width seems fine.
       */
      Dimension maxWidth = (WidthOfScreen(app->screen) * 8 / 10);
      Dimension extraSpace = ((2 * d->w3.horizontalSpacing) +
			      (2 * d->w3.shadowThickness));
      
      if (d->indicator.maximumCount < 8) {
	 d->indicator.maximumCount = 8;
      }
      if (((d->indicator.maximumCount * d->indicator.w3.w.width) +
	   ((d->indicator.maximumCount - 1) *
	    d->indicator.w3.horizontalSpacing) + extraSpace) > maxWidth) {
	 d->indicator.maximumCount =
	    ((maxWidth - extraSpace - d->indicator.w3.w.width) /
	     (d->indicator.w3.w.width + d->indicator.w3.horizontalSpacing))
	    + 1;
      }
      if (d->indicator.minimumCount <= 6) {
	 d->indicator.minimumCount = 6;
      }
      if (d->indicator.minimumCount > d->indicator.maximumCount) {
	 d->indicator.minimumCount = d->indicator.maximumCount;
      }
   }
   
   {
      /* Calculate the width and horizontal position of things. */
      Dimension labelAreaWidth;
      Dimension buttonAreaWidth;
      Dimension indicatorAreaWidth;
      Dimension extraIndicatorSpace;
      Dimension singleIndicatorSpace;
      Dimension interButtonSpace;
      Dimension w;
      Position leftX;
      int i;
      
      labelAreaWidth = d->label.w.width + (2 * d->w3.horizontalSpacing);
      buttonAreaWidth = ((3 * d->w3.horizontalSpacing) +
			 d->okButton.w3.w.width +
			 d->cancelButton.w3.w.width);
      w = MAX(labelAreaWidth, buttonAreaWidth);
      extraIndicatorSpace = ((2 * d->w3.horizontalSpacing) +
			     d->indicator.w3.w.width);
      singleIndicatorSpace = (d->indicator.w3.w.width +
			      d->indicator.w3.horizontalSpacing);
      d->indicator.count = ((w - extraIndicatorSpace) / singleIndicatorSpace);
      d->indicator.current = 0;
      d->indicator.count++; /* For gatepost indicator in extra space. */
      if (((w - extraIndicatorSpace) % singleIndicatorSpace) >
	  (singleIndicatorSpace / 2)) {
	 d->indicator.count++;
      }
      if (d->indicator.count < d->indicator.minimumCount) {
	 d->indicator.count = d->indicator.minimumCount;
      }
      if (d->indicator.count > d->indicator.maximumCount) {
	 d->indicator.count = d->indicator.maximumCount;
      }
      indicatorAreaWidth = ((singleIndicatorSpace * (d->indicator.count - 1)) +
			    extraIndicatorSpace);
      d->w3.interiorWidth = MAX(w, indicatorAreaWidth);
      d->w3.w.width = d->w3.interiorWidth + (2 * d->w3.shadowThickness);

      leftX = (d->w3.w.width - d->label.w.width) / 2;
      d->label.w.x = leftX;
      
      leftX = ((d->w3.w.width -
	       (d->indicator.count * d->indicator.w3.w.width) -
	       ((d->indicator.count - 1) * d->indicator.w3.horizontalSpacing))
	       / 2);
      {
	 int n = d->indicator.count * sizeof(IndicatorElement);
	 d->indicators = malloc(n);
	 if (NULL == d->indicators) {
	    destroyDialog(app);
	    outOfMemory(app, __LINE__);
	 }
	 memset(d->indicators, 0, n);
      }
      d->indicators[0].parent = &(d->indicator);
      d->indicators[0].w.x = d->indicator.w3.w.x = leftX;
      d->indicators[0].w.width = d->indicator.w3.w.width;
      d->indicators[0].isLit = False;
      for (i = 1; i < d->indicator.count; i++) {
	 d->indicators[i].parent = &(d->indicator);
	 d->indicators[i].w.x = (d->indicators[i - 1].w.x +
				 d->indicator.w3.w.width +
				 d->indicator.w3.horizontalSpacing);
	 d->indicators[i].w.width = d->indicator.w3.w.width;
	 d->indicators[i].isLit = False;
      }
      interButtonSpace = ((d->w3.interiorWidth - d->okButton.w3.w.width -
			   d->cancelButton.w3.w.width) / 3);
      d->okButton.w3.w.x = interButtonSpace + d->w3.shadowThickness;
      d->cancelButton.w3.w.x = (d->okButton.w3.w.x + d->okButton.w3.w.width +
				interButtonSpace);
   }
   {
      /* Calculate the height and vertical position of things. */
      int i;
      
      d->w3.interiorHeight = ((4 * d->w3.verticalSpacing) +
			      (2 * d->indicator.w3.verticalSpacing) +
			      d->label.w.height +
			      d->indicator.w3.w.height +
			      d->okButton.w3.w.height);
      d->w3.w.height = d->w3.interiorHeight + (2 * d->w3.shadowThickness);
      d->label.w.y = (d->w3.shadowThickness + d->w3.verticalSpacing +
		      d->label.ascent);
      d->indicator.w3.w.y = (d->label.w.y + d->label.descent +
			     d->w3.verticalSpacing +
			     d->indicator.w3.verticalSpacing);
      for (i = 0; i < d->indicator.count; i++) {
	 d->indicators[i].w.y = d->indicator.w3.w.y;
	 d->indicators[i].w.height = d->indicator.w3.w.height;
      }
      d->okButton.w3.w.y = d->cancelButton.w3.w.y =
	 (d->indicator.w3.w.y + d->indicator.w3.w.height +
	  d->w3.verticalSpacing + d->indicator.w3.verticalSpacing);
   }
   calcButtonLabelPosition(&(d->okButton));
   calcButtonLabelPosition(&(d->cancelButton));

   d->w3.w.x = (WidthOfScreen(app->screen) - d->w3.w.width) / 2;
   d->w3.w.y = (HeightOfScreen(app->screen) - d->w3.w.height) / 3;
   
   app->dialog = d;
}

void destroyDialog(AppInfo *app)
{
   DialogInfo *d = app->dialog;
   
   freeIf(d->title);
   freeIf(d->label.text);
   freeIf(d->okButton.label.text);
   freeIf(d->cancelButton.label.text);
   freeIf(d->indicators);
   
   freeFontIf(app, d->label.font);
   freeFontIf(app, d->okButton.label.font);
   freeFontIf(app, d->cancelButton.label.font);
   
   XFree(d->sizeHints);
   XFree(d->wmHints);
   XFree(d->classHints);
   XFree(d->windowName.value);
   
   freeIf(d);
}

void createDialogWindow(AppInfo *app)
{
   XSetWindowAttributes attr;
   unsigned long attrMask = 0;
   DialogInfo *d = app->dialog;
   
   attr.background_pixel = d->w3.w.background;
   attrMask |= CWBackPixel;
   attr.border_pixel = d->w3.borderColor;
   attrMask |= CWBorderPixel;
   attr.cursor = None;
   attrMask |= CWCursor;
   attr.event_mask = 0;
   attr.event_mask |= ExposureMask;
   attr.event_mask |= ButtonPressMask;
   attr.event_mask |= ButtonReleaseMask;
   attr.event_mask |= KeyPressMask;
   attrMask |= CWEventMask;

   d->dialogWindow = XCreateWindow(app->dpy, app->rootWindow,
				   d->w3.w.x, d->w3.w.y,
				   d->w3.w.width, d->w3.w.height,
				   d->w3.borderWidth,
				   DefaultDepthOfScreen(app->screen),
				   InputOutput,
				   DefaultVisualOfScreen(app->screen),
				   attrMask, &attr);
   
   d->sizeHints = XAllocSizeHints();
   if (!(d->sizeHints)) {
      destroyDialog(app);
      outOfMemory(app, __LINE__);
   }
   d->sizeHints->flags = 0;
   d->sizeHints->flags |= PPosition;
   d->sizeHints->flags |= PSize;
   d->sizeHints->min_width = d->w3.w.width;
   d->sizeHints->min_height = d->w3.w.height;
   d->sizeHints->flags |= PMinSize;
   d->sizeHints->max_width = d->w3.w.width;
   d->sizeHints->max_height = d->w3.w.height;
   d->sizeHints->flags |= PMaxSize;
   d->sizeHints->base_width = d->w3.w.width;
   d->sizeHints->base_height = d->w3.w.height;
   d->sizeHints->flags |= PBaseSize;
   
   d->wmHints = XAllocWMHints();
   if (!(d->wmHints)) {
      destroyDialog(app);
      outOfMemory(app, __LINE__);
   }
   d->wmHints->flags = 0;
   d->wmHints->input = True;
   d->wmHints->flags |= InputHint;
   d->wmHints->initial_state = NormalState;
   d->wmHints->flags |= StateHint;

   d->classHints = XAllocClassHint();
   if (!(d->classHints)) {
      destroyDialog(app);
      outOfMemory(app, __LINE__);
   }
   d->classHints->res_name = app->appName;
   d->classHints->res_class = app->appClass;

   if (!XStringListToTextProperty(&(d->title), 1, &(d->windowName))) {
      destroyDialog(app);
      outOfMemory(app, __LINE__);
   }
   XSetWMProperties(app->dpy, d->dialogWindow, &(d->windowName), NULL,
		    app->argv, app->argc, d->sizeHints,
		    d->wmHints, d->classHints);
   XSetTransientForHint(app->dpy, d->dialogWindow, d->dialogWindow);
   
   app->wmDeleteWindowAtom = XInternAtom(app->dpy, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(app->dpy, d->dialogWindow, &(app->wmDeleteWindowAtom), 1);
}

void createGCs(AppInfo *app)
{
   DialogInfo *d = app->dialog;
   
   XGCValues gcv;
   unsigned long gcvMask;
   
   gcvMask = 0;
   gcv.foreground = d->w3.w.background;
   gcvMask |= GCForeground;
   gcv.fill_style = FillSolid;
   gcvMask |= GCFillStyle;
   app->fillGC = XCreateGC(app->dpy, app->rootWindow, gcvMask, &gcv);
   
   gcvMask = 0;
   gcv.foreground = d->w3.borderColor;
   gcvMask |= GCForeground;
   gcv.line_width = d->w3.borderWidth;
   gcvMask |= GCLineWidth;
   gcv.line_style = LineSolid;
   gcvMask |= GCLineStyle;
   gcv.cap_style = CapButt;
   gcvMask |= GCCapStyle;
   gcv.join_style = JoinMiter;
   gcvMask |= GCJoinStyle;
   app->borderGC = XCreateGC(app->dpy, app->rootWindow, gcvMask, &gcv);
   
   gcvMask = 0;
   gcv.foreground = d->label.w.foreground;
   gcvMask |= GCForeground;
   gcv.background = d->label.w.background;
   gcvMask |= GCBackground;
   gcv.font = d->label.font->fid;
   gcvMask |= GCFont;
   app->textGC = XCreateGC(app->dpy, app->rootWindow, gcvMask, &gcv);
   
   gcvMask = 0;
   gcv.foreground = d->indicator.w3.w.foreground;
   gcvMask |= GCForeground;
   gcv.fill_style = FillSolid;
   gcvMask |= GCFillStyle;
   app->brightGC = XCreateGC(app->dpy, app->rootWindow, gcvMask, &gcv);
   
   gcvMask = 0;
   gcv.foreground = d->indicator.w3.w.background;
   gcvMask |= GCForeground;
   gcv.fill_style = FillSolid;
   gcvMask |= GCFillStyle;
   app->dimGC = XCreateGC(app->dpy, app->rootWindow, gcvMask, &gcv);
}

void destroyGCs(AppInfo *app)
{
   XFreeGC(app->dpy, app->fillGC);
   XFreeGC(app->dpy, app->borderGC);
   XFreeGC(app->dpy, app->textGC);
   XFreeGC(app->dpy, app->brightGC);
   XFreeGC(app->dpy, app->dimGC);
}

void paintLabel(AppInfo *app, Drawable draw, LabelInfo label)
{
   if (!(label.text)) {
      return;
   }
   XSetForeground(app->dpy, app->textGC, label.w.foreground);
   XSetBackground(app->dpy, app->textGC, label.w.background);
   XSetFont(app->dpy, app->textGC, label.font->fid);
   XDrawString(app->dpy, draw, app->textGC, label.w.x, label.w.y, label.text,
	       label.textLength);
}

void paintButton(AppInfo *app, Drawable draw, ButtonInfo button)
{
   Position x;
   Position y;
   Dimension width;
   Dimension height;
   
   if (button.w3.borderWidth > 0) {
      XSetForeground(app->dpy, app->borderGC, button.w3.borderColor);
      XFillRectangle(app->dpy, draw, app->borderGC, button.w3.w.x,
		     button.w3.w.y, button.w3.w.width, button.w3.w.height);
   }
   if ((button.w3.shadowThickness <= 0) && (button.pressed)) {
      Pixel tmp = button.w3.w.background;
      button.w3.w.background = button.w3.w.foreground;
      button.w3.w.foreground = tmp;
      tmp = button.label.w.background;
      button.label.w.background = button.label.w.foreground;
      button.label.w.foreground = tmp;
   }
   x = (button.w3.w.x + button.w3.borderWidth);
   y = (button.w3.w.y + button.w3.borderWidth);
   width = (button.w3.w.width - (2 * button.w3.borderWidth));
   height = (button.w3.w.height - (2 * button.w3.borderWidth));
   if ((button.w3.shadowThickness > 0) && (button.pressed)) {
      XSetForeground(app->dpy, app->fillGC, button.w3.topShadowColor);
   } else {
      XSetForeground(app->dpy, app->fillGC, button.w3.w.background);
   }
   XFillRectangle(app->dpy, draw, app->fillGC, x, y, width, height);
   if (button.w3.shadowThickness > 0) {
      if (button.pressed) {
	 draw_shaded_rectangle(app->dpy, draw, x, y, width, height,
			       button.w3.shadowThickness,
			       button.w3.bottomShadowColor,
			       button.w3.topShadowColor);
      } else {
	 draw_shaded_rectangle(app->dpy, draw, x, y, width, height,
			       button.w3.shadowThickness,
			       button.w3.topShadowColor,
			       button.w3.bottomShadowColor);
      }
   }
   paintLabel(app, draw, button.label);
   if ((button.w3.shadowThickness <= 0) && (button.pressed)) {
      Pixel tmp = button.w3.w.background;
      button.w3.w.background = button.w3.w.foreground;
      button.w3.w.foreground = tmp;
      tmp = button.label.w.background;
      button.label.w.background = button.label.w.foreground;
      button.label.w.foreground = tmp;
   }
}

void paintIndicator(AppInfo *app, Drawable draw, IndicatorElement indicator)
{
   Position x;
   Position y;
   Dimension width;
   Dimension height;
   GC gc = app->dimGC;
   
   if (indicator.parent->w3.borderWidth > 0) {
      XSetForeground(app->dpy, app->borderGC,
		     indicator.parent->w3.borderColor);
      XFillRectangle(app->dpy, draw, app->borderGC, indicator.w.x,
		     indicator.w.y, indicator.w.width, indicator.w.height);
   }
   if (indicator.isLit) {
      gc = app->brightGC;
   }
   x = (indicator.w.x + indicator.parent->w3.borderWidth);
   y = (indicator.w.y + indicator.parent->w3.borderWidth);
   width = (indicator.w.width - (2 * indicator.parent->w3.borderWidth));
   height = (indicator.w.height - (2 * indicator.parent->w3.borderWidth));
   XFillRectangle(app->dpy, draw, gc, x, y, width, height);
   if (indicator.parent->w3.shadowThickness > 0) {
      draw_shaded_rectangle(app->dpy, draw, x, y, width, height,
			    indicator.parent->w3.shadowThickness,
			    indicator.parent->w3.bottomShadowColor,
			    indicator.parent->w3.topShadowColor);
   }
}

void updateIndicatorElement(AppInfo *app, int i)
{
   DialogInfo *d = app->dialog;
   
   d->indicators[i].isLit = !(d->indicators[i].isLit);
   paintIndicator(app, d->dialogWindow, d->indicators[i]);
}

void updateIndicators(AppInfo *app, int condition)
{
   DialogInfo *d = app->dialog;
   
   if (condition > 0) {
      /* Move forward one. */
      updateIndicatorElement(app, d->indicator.current);
      if (d->indicator.current < (d->indicator.count - 1)) {
	 (d->indicator.current)++;
      } else {
	 d->indicator.current = 0;
      }
   } else if (condition < 0) {
      /* Move backward one. */
      if (d->indicator.current > 0) {
	 (d->indicator.current)--;
      } else {
	 d->indicator.current = d->indicator.count - 1;
      }
      updateIndicatorElement(app, d->indicator.current);
   } else {
      /* Erase them all. */
      int i;
      
      for (i = 0; i < d->indicator.count; i++) {
	 d->indicators[i].isLit = False;
	 paintIndicator(app, d->dialogWindow, d->indicators[i]);
      }
      d->indicator.current = 0;
   }
   XSync(app->dpy, False);
}

void paintDialog(AppInfo *app)
{
   DialogInfo *d = app->dialog;
   Drawable draw = d->dialogWindow;
   int i;
   
   XSetForeground(app->dpy, app->fillGC, d->w3.w.background);
   XFillRectangle(app->dpy, draw, app->fillGC, 0, 0,
		  d->w3.w.width, d->w3.w.height);
   if (d->w3.shadowThickness > 0) {
      draw_shaded_rectangle(app->dpy, draw, 0, 0,
			    d->w3.w.width, d->w3.w.height,
			    d->w3.shadowThickness,
			    d->w3.topShadowColor,
			    d->w3.bottomShadowColor);
   }
   paintLabel(app, draw, d->label);
   for (i = 0; i < d->indicator.count; i++) {
      paintIndicator(app, draw, d->indicators[i]);
   }
   paintButton(app, draw, d->okButton);
   paintButton(app, draw, d->cancelButton);
   XSync(app->dpy, False);
}

void grabKeyboard(AppInfo *app)
{
   if ((!(app->grabKeyboard)) || (app->isKeyboardGrabbed)) {
      return;
   } else {
      int status;
      Window grabWindow = app->dialog->dialogWindow;
      Bool ownerEvents = False;
      Bool pointerMode = GrabModeAsync;
      Bool keyboardMode = GrabModeAsync;

      app->isKeyboardGrabbed = True;
      XSync(app->dpy, False);
      status = XGrabKeyboard(app->dpy, grabWindow, ownerEvents,
			     pointerMode, keyboardMode, CurrentTime);
      XSync(app->dpy, False);
      if (GrabSuccess != status) {
	 char *reason = "reason unknown";
	 
	 switch (status) {
	  case AlreadyGrabbed:
	    reason = "someone else already has the keyboard";
	    break;
	  case GrabFrozen:
	    reason = "someone else has frozen the keyboard";
	    break;
	  case GrabInvalidTime:
	    reason = "bad grab time [this shouldn't happen]";
	    break;
	  case GrabNotViewable:
	    reason = "grab not viewable [this shouldn't happen]";
	    break;
	 }
	 fprintf(stderr, "%s: Could not grab keyboard (%s)\n", app->appName);
	 exitApp(app, EXIT_STATUS_ERROR);
      }
   }
}

void ungrabKeyboard(AppInfo *app)
{
   if (app->grabKeyboard) {
      XUngrabKeyboard(app->dpy, CurrentTime);
   }
}

void grabPointer(AppInfo *app)
{
   if ((!(app->grabPointer)) || (app->isPointerGrabbed)) {
      return;
   } else {
      int status;
      Window grabWindow = app->dialog->dialogWindow;
      Bool ownerEvents = False;
      unsigned int eventMask = ButtonPressMask | ButtonReleaseMask;
      Bool pointerMode = GrabModeAsync;
      Bool keyboardMode = GrabModeAsync;
      Window confineTo = None;
      Cursor cursor = None;

      app->isPointerGrabbed = True;
      XSync(app->dpy, False);
      status = XGrabPointer(app->dpy, grabWindow, ownerEvents, eventMask,
			    pointerMode, keyboardMode, confineTo, cursor,
			    CurrentTime);
      XSync(app->dpy, False);
      if (GrabSuccess != status) {
	 char *reason = "reason unknown";
	 
	 switch (status) {
	  case AlreadyGrabbed:
	    reason = "someone else already has the pointer";
	    break;
	  case GrabFrozen:
	    reason = "someone else has frozen the pointer";
	    break;
	  case GrabInvalidTime:
	    reason = "bad grab time [this shouldn't happen]";
	    break;
	  case GrabNotViewable:
	    reason = "grab not viewable [this shouldn't happen]";
	    break;
	 }
	 fprintf(stderr, "%s: Could not grab pointer (%s)\n", app->appName);
	 exitApp(app, EXIT_STATUS_ERROR);
      }
   }
}

void ungrabPointer(AppInfo *app)
{
   if (app->grabPointer) {
      XUngrabPointer(app->dpy, CurrentTime);
   }
}

void grabServer(AppInfo *app)
{
   if ((!(app->grabServer)) || (app->isServerGrabbed)) {
      return;
   } else {
      app->isServerGrabbed = True;
      XSync(app->dpy, False);
      XGrabServer(app->dpy);
      XSync(app->dpy, False);
   }
}

void ungrabServer(AppInfo *app)
{
   if (app->grabServer) {
      XUngrabServer(app->dpy);
   }
}

void cleanUp(AppInfo *app)
{
   XDestroyWindow(app->dpy, app->dialog->dialogWindow);
   destroyGCs(app);
   destroyDialog(app);
   if (app->buf) {
      memset(app->buf, 0, app->bufSize);
   }
   freeIf(app->buf);
   ungrabPointer(app);
   ungrabKeyboard(app);
   ungrabServer(app);
}

void exitApp(AppInfo *app, int exitCode)
{
   cleanUp(app);
   exit(exitCode);
}

void acceptAction(AppInfo *app)
{
   int status = append_to_buf(&(app->buf), &(app->bufSize),
			      &(app->bufIndex), '\0');
   if (APPEND_FAILURE == status) {
      cleanUp(app);
      outOfMemory(app, __LINE__);
   }
   fputs(app->buf, stdout);
   fputc('\n', stdout);
   exitApp(app, EXIT_STATUS_ACCEPT);
}

void cancelAction(AppInfo *app)
{
   exitApp(app, EXIT_STATUS_CANCEL);
}

void backspacePassphrase(AppInfo *app)
{
   if (0 >= app->bufIndex) {
      XBell(app->dpy, 0);
      return;
   }
   (app->bufIndex)--;
   updateIndicators(app, -1);
}

void erasePassphrase(AppInfo *app)
{
   if (0 >= app->bufIndex) {
      XBell(app->dpy, 0);
      return;
   }
   updateIndicators(app, 0);
   app->bufIndex = 0;
}

void addToPassphrase(AppInfo *app, char c)
{
   int status = append_to_buf(&(app->buf), &(app->bufSize),
			      &(app->bufIndex), c);
   if (APPEND_FAILURE == status) {
      cleanUp(app);
      outOfMemory(app, __LINE__);
   }
   updateIndicators(app, 1);
}

void handleKeyPress(AppInfo *app, XKeyEvent *event)
{
   char s[2];
   int n;
   
   if (event->send_event) {
      /* Pay no attention to synthetic key events. */
      return;
   }
   n = XLookupString(event, s, 1, NULL, NULL);
   
   if (1 != n) {
      return;
   }
   s[1] = '\0';
   switch (s[0]) {
    case '\010':
    case '\177':
      backspacePassphrase(app);
      break;
    case '\025':
    case '\030':
      erasePassphrase(app);
      break;
    case '\012':
    case '\015':
      acceptAction(app);
      break;
    case '\033':
      cancelAction(app);
      break;
    default:
      addToPassphrase(app, s[0]);
      break;
   }
}

Bool eventIsInsideButton(AppInfo *app, XButtonEvent *event, ButtonInfo button)
{
   int status = False;
   
   if ((event->x >= (button.w3.w.x + button.w3.borderWidth)) &&
       (event->x < (button.w3.w.x + button.w3.w.width -
		    (2 * button.w3.borderWidth))) &&
       (event->y >= (button.w3.w.y + button.w3.borderWidth)) &&
       (event->y < (button.w3.w.y + button.w3.w.height -
		    (2 * button.w3.borderWidth)))) {
      status = True;
   }
   return(status);
}

void handleButtonPress(AppInfo *app, XButtonEvent *event)
{
   DialogInfo *d = app->dialog;
   
   if (event->button != Button1) {
      return;
   }
   if (ButtonPress == event->type) {
      if (eventIsInsideButton(app, event, d->okButton)) {
	 d->pressedButton = OK_BUTTON;
	 d->okButton.pressed = True;
	 paintButton(app, d->dialogWindow, d->okButton);
      } else if (eventIsInsideButton(app, event, d->cancelButton)) {
	 d->pressedButton = CANCEL_BUTTON;
	 d->cancelButton.pressed = True;
	 paintButton(app, d->dialogWindow, d->cancelButton);
      } else {
	 d->pressedButton = NO_BUTTON;
      }
   } else if (ButtonRelease == event->type) {
      if (OK_BUTTON == d->pressedButton) {
	 if (eventIsInsideButton(app, event, d->okButton)) {
	    acceptAction(app);
	 } else {
	    d->okButton.pressed = False;
	    paintButton(app, d->dialogWindow, d->okButton);
	 }
      } else if (CANCEL_BUTTON == d->pressedButton) {
	 if (eventIsInsideButton(app, event, d->cancelButton)) {
	    cancelAction(app);
	 } else {
	    d->cancelButton.pressed = False;
	    paintButton(app, d->dialogWindow, d->cancelButton);
	 }
      }
      d->pressedButton = NO_BUTTON;
   }
}

int main(int argc, char **argv)
{
   AppInfo app;
   XEvent event;

   memset(&app, 0, sizeof(app));
   
   app.argc = argc;
   app.argv = argv;

   progclass = "SshAskpass";
   app.toplevelShell = XtAppInitialize(&(app.appContext), progclass,
					NULL, 0, &argc, argv,
					defaults, NULL, 0);
   app.dpy = XtDisplay(app.toplevelShell);
   app.screen = DefaultScreenOfDisplay(app.dpy);
   app.rootWindow = RootWindowOfScreen(app.screen);
   app.black = BlackPixel(app.dpy, DefaultScreen(app.dpy));
   app.white = WhitePixel(app.dpy, DefaultScreen(app.dpy));
   app.colormap = DefaultColormapOfScreen(app.screen);
   app.resourceDb = XtDatabase(app.dpy);
   XtGetApplicationNameAndClass(app.dpy, &progname, &progclass);
   app.appName = progname;
   app.appClass = progclass;
   /* For resources.c. */
   db = app.resourceDb;

   {
      struct rlimit resourceLimit;
      int status;
      
      status = getrlimit(RLIMIT_CORE, &resourceLimit);
      if (-1 == status) {
	 fprintf(stderr, "%s: getrlimit failed (%s)\n", app.appName,
		 strerror(errno));
	 exit(EXIT_STATUS_ERROR);
      }
      resourceLimit.rlim_cur = 0;
      status = setrlimit(RLIMIT_CORE, &resourceLimit);
      if (-1 == status) {
	 fprintf(stderr, "%s: setrlimit failed (%s)\n", app.appName,
		 strerror(errno));
	 exit(EXIT_STATUS_ERROR);
      }
   }

   createDialog(&app);
   createGCs(&app);
   createDialogWindow(&app);
   
   XMapWindow(app.dpy, app.dialog->dialogWindow);
   
   while(True) {
      XNextEvent(app.dpy, &event);
      switch (event.type) {
       case Expose:
	 grabServer(&app);
	 grabKeyboard(&app);
	 grabPointer(&app);
	 if (event.xexpose.count) {
	    break;
	 }
	 paintDialog(&app);
	 break;
       case ButtonPress:
       case ButtonRelease:
	 handleButtonPress(&app, &(event.xbutton));
	 break;
       case KeyPress:
	 handleKeyPress(&app, &(event.xkey));
	 break;
       case ClientMessage:
	 if ((32 == event.xclient.format) &&
	     (event.xclient.data.l[0] == app.wmDeleteWindowAtom)) {
	    cancelAction(&app);
	 }
	 break;
       default:
	 break;
      }
   }

   fprintf(stderr, "%s: This should not happen.\n", app.appName);
   return(EXIT_STATUS_ANOMALY);
}

