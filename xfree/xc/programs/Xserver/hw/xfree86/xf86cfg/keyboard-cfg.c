/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/xf86cfg/keyboard-cfg.c,v 1.10 2000/12/11 18:47:46 paulo Exp $
 */

#include "xf86config.h"
#include "keyboard-cfg.h"
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>

/*
 * Prototypes
 */
static void KeyboardModelCallback(Widget, XtPointer, XtPointer);
static void KeyboardLayoutCallback(Widget, XtPointer, XtPointer);
static void KeyboardApplyCallback(Widget, XtPointer, XtPointer);
static Bool KeyboardConfigCheck(void);
static void XkbUIEventHandler(Widget, XtPointer, XEvent*, Boolean*);

/*
 * Initialization
 */
static XF86XkbDescInfo xkb_model;
static XF86XkbDescInfo xkb_layout;
static XF86XkbDescInfo xkb_variant;
static XF86XkbDescInfo xkb_option;
#ifdef XFREE98_XKB
static char *XkbRulesFile = "lib/X11/xkb/rules/xfree98";
#else
static char *XkbRulesFile = "lib/X11/xkb/rules/xfree86";
#endif
static XF86ConfInputPtr current_input;

static char *model, *layout;
static Widget kbd, modelb, layoutb;
static XkbInfo **xkb_infos;
static int num_xkb_infos;
XkbInfo *xkb_info;

static Widget apply;

/*
 * Implementation
 */
/*ARGSUSED*/
XtPointer
KeyboardConfig(XtPointer config)
{
    XF86ConfInputPtr keyboard = (XF86ConfInputPtr)config;
    XF86OptionPtr option;
    Arg args[1];
    static char *XkbModel = "XkbModel", *XkbLayout = "XkbLayout";

    InitializeKeyboard();

    if (xkb_info->conf == NULL)
	xkb_info->conf = keyboard;

    if (xkb_info->conf != keyboard) {
	int i;

	for (i = 0; i < num_xkb_infos; i++)
	    if (xkb_infos[i]->conf == keyboard) {
		xkb_info = xkb_infos[i];
		break;
	    }

	if (i >= num_xkb_infos) {
	    int timeout = 10;

	    xkb_info = (XkbInfo*)XtCalloc(1, sizeof(XkbInfo));
	    xkb_info->conf = keyboard;
	    xkb_infos = (XkbInfo**)
		XtRealloc((XtPointer)xkb_infos, sizeof(XkbInfo*) *
			  (num_xkb_infos + 1));
	    xkb_infos[num_xkb_infos++] = xkb_info;

	    xkb_info->conf = keyboard;
	    while (timeout > 0) {
		xkb_info->xkb =
		    XkbGetKeyboard(XtDisplay(configp),
				   XkbGBN_AllComponentsMask, XkbUseCoreKbd);
		if (xkb_info->xkb == NULL || xkb_info->xkb->geom == NULL)
		    sleep(timeout -= 1);
		else
		    break;
	    }
	    if (timeout <= 0) {
		fprintf(stderr, "Couldn't get keyboard\n");
		exit(1);
	    }
	    if (xkb_info->xkb->names->geometry == 0)
		xkb_info->xkb->names->geometry = xkb_info->xkb->geom->name;

	    bzero((char*)&(xkb_info->defs), sizeof(XkbRF_VarDefsRec));
	}

	/* check for removed devices */
	for (i = 0; i < num_xkb_infos; i++) {
	    XF86ConfInputPtr key = XF86Config->conf_input_lst;

	    while (key != NULL) {
		if (strcasecmp(key->inp_driver, "keyboard") == 0 &&
		    xkb_infos[i]->conf == key)
		    break;
		key = (XF86ConfInputPtr)(key->list.next);
	    }
	    if (xkb_infos[i]->conf != NULL && key == NULL) {
		XkbFreeKeyboard(xkb_infos[i]->xkb, 0, False);
		XtFree((XtPointer)xkb_infos[i]);
		if (--num_xkb_infos > i)
		    memmove(&xkb_infos[i], &xkb_infos[i + 1],
			(num_xkb_infos - i) * sizeof(XkbInfo*));
	    }
	}
    }

    current_input = keyboard;

    if (keyboard != NULL) {
	if ((option = xf86findOption(keyboard->inp_option_lst, XkbModel)) != NULL)
	    xkb_info->defs.model = model = option->opt_val;
	else
	    xkb_info->defs.model = model = xkb_model.name[0];
	if ((option = xf86findOption(keyboard->inp_option_lst, XkbLayout)) != NULL)
	    xkb_info->defs.layout = layout = option->opt_val;
	else
	    xkb_info->defs.layout = layout = xkb_layout.name[0];

	XtSetArg(args[0], XtNstring, keyboard->inp_identifier);
	XtSetValues(ident_widget, args, 1);

	UpdateKeyboard(False);
    }
    else {
	XF86ConfInputPtr input = XF86Config->conf_input_lst;
	char keyboard_name[48];
	int nkeyboards = 0;

	while (input != NULL) {
	    if (strcasecmp(input->inp_driver, "keyboard") == 0)
		++nkeyboards;
	    input = (XF86ConfInputPtr)(input->list.next);
	}
	do {
	    ++nkeyboards;
	    XmuSnprintf(keyboard_name, sizeof(keyboard_name),
			"Keyboard%d", nkeyboards);
	} while (xf86findInput(keyboard_name,
		 XF86Config->conf_input_lst));

	model = xkb_model.name[0];
	layout = xkb_layout.name[0];
	XtSetArg(args[0], XtNstring, keyboard_name);
	XtSetValues(ident_widget, args, 1);
    }

    xf86info.cur_list = KEYBOARD;
    XtSetSensitive(back, xf86info.lists[KEYBOARD].cur_function > 0);
    XtSetSensitive(next, xf86info.lists[KEYBOARD].cur_function <
			 xf86info.lists[KEYBOARD].num_functions - 1);
    (xf86info.lists[KEYBOARD].functions[xf86info.lists[KEYBOARD].cur_function])
	(&xf86info);

    if (ConfigLoop(KeyboardConfigCheck) == True) {
	if (keyboard == NULL) {
	    keyboard = XtNew(XF86ConfInputRec);
	    keyboard->list.next = NULL;
	    keyboard->inp_identifier = XtNewString(ident_string);
	    keyboard->inp_driver = XtNewString("keyboard");
	    keyboard->inp_option_lst = xf86newOption(XtNewString(XkbModel),
						     XtNewString(model));
	    xf86addNewOption(keyboard->inp_option_lst,
			     XtNewString(XkbLayout), XtNewString(layout));
	    keyboard->inp_comment = NULL;
	}
	else {
	    int i;
	    char *str;

	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(modelb, args, 1);
	    for (i = 0; i < xkb_model.nelem; i++)
		if (strcmp(xkb_model.desc[i], str) == 0) {
		    model = xkb_model.name[i];
		    break;
		}

	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(layoutb, args, 1);
	    for (i = 0; i < xkb_layout.nelem; i++)
		if (strcmp(xkb_layout.desc[i], str) == 0) {
		    layout = xkb_layout.name[i];
		    break;
		}

	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbModel))
		!= NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(model);
		XtFree(option->opt_comment);
	    }
	    else {
		if (keyboard->inp_option_lst == NULL)
		    keyboard->inp_option_lst = xf86newOption(XtNewString(XkbModel),
							     XtNewString(model));
		else
		    xf86addNewOption(keyboard->inp_option_lst,
				     XtNewString(XkbModel), XtNewString(model));
	    }
	    XtFree(xkb_info->config.model);
	    xkb_info->config.model = XtNewString(model);

	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbLayout))
		!= NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(layout);
		XtFree(option->opt_comment);
	    }
	    else
		xf86addNewOption(keyboard->inp_option_lst,
				 XtNewString(XkbLayout), XtNewString(layout));
	    XtFree(xkb_info->config.layout);
	    xkb_info->config.layout = XtNewString(layout);
	}
	if (strcasecmp(keyboard->inp_identifier, ident_string))
	    xf86renameInput(XF86Config, keyboard, ident_string);

	xkb_info->conf = keyboard;
	return ((XtPointer)keyboard);
    }

    return (NULL);
}

static Bool
KeyboardConfigCheck(void)
{
    XF86ConfInputPtr keyboard = XF86Config->conf_input_lst;

    while (keyboard != NULL) {
	if (keyboard != current_input &&
	    strcasecmp(ident_string, keyboard->inp_identifier) == 0)
	    return (False);
	keyboard = (XF86ConfInputPtr)(keyboard->list.next);
    }

    return (True);
}

/*ARGSUSED*/
static void
XkbUIEventHandler(Widget w, XtPointer closure,
		  XEvent *event, Boolean *continue_to_dispatch)
{
    XkbUI_ViewOptsRec opts;
    XkbUI_ViewPtr view;
    int width, height, bd;

    if (event->xexpose.count > 1)
	return;

    bzero((char *)&opts, sizeof(opts));
    bd = 1;
    opts.present = XkbUI_SizeMask | XkbUI_ColormapMask |
		   XkbUI_MarginMask | XkbUI_OffsetMask;
    opts.margin_width = opts.margin_height = 0;
    opts.viewport.x = opts.viewport.y = bd;
    width = opts.viewport.width = w->core.width - 2 * bd;
    height = opts.viewport.height = w->core.height - 2 * bd;
    opts.cmap = w->core.colormap;

    if ((view = XkbUI_Init(XtDisplay(w), XtWindow(w), width, height,
	xkb_info->xkb, &opts)) != NULL) {
	XkbUI_DrawRegion(view, NULL);
	free(view);
    }
}

void
InitializeKeyboard(void)
{
    int major, minor, op, event, error;
    static int first = 1;
    XkbRF_RulesPtr list;
    int i, timeout = 5;
    XF86ConfInputPtr keyboard = XF86Config->conf_input_lst;
    XF86OptionPtr option;
    char name[PATH_MAX];
    FILE *file;

    if (!first)
	return;
    first = 0;

    major = XkbMajorVersion;
    minor = XkbMinorVersion;
    if (XkbQueryExtension(DPY, &op, &event, &error, &major, &minor) == 0) {
	fprintf(stderr, "Unable to initialize XKEYBOARD extension");
	exit(1);
    }

    xkb_info = (XkbInfo *)XtCalloc(1, sizeof(XkbInfo));
    xkb_info->conf = NULL;
    xkb_infos = (XkbInfo**)XtCalloc(1, sizeof(XkbInfo*));
    num_xkb_infos = 1;
    xkb_infos[0] = xkb_info;

    while (timeout > 0) {
	xkb_info->xkb =
	    XkbGetKeyboard(DPY, XkbGBN_AllComponentsMask, XkbUseCoreKbd);
	if (xkb_info->xkb == NULL || xkb_info->xkb->geom == NULL) {
	    timeout -= 1;
	    sleep(1);
	}
	else
	    break;
    }
    if (timeout <= 0) {
	fprintf(stderr, "Couldn't get keyboard\n");
	exit(1);
    }
    if (xkb_info->xkb->names->geometry == 0)
	xkb_info->xkb->names->geometry = xkb_info->xkb->geom->name;

    bzero((char*)&(xkb_info->defs), sizeof(XkbRF_VarDefsRec));

    if ((list = XkbRF_Create(0, 0)) == NULL ||
	!XkbRF_LoadDescriptionsByName(XkbRulesFile, NULL, list)) {
	fprintf(stderr, "Can't create rules structure\n");
	exit(1);
    }

    for (i = 0; i < list->models.num_desc; i++) {
	if (i % 16 == 0) {
	    xkb_model.name = (char**)XtRealloc((XtPointer)xkb_model.name,
					       (i + 16) * sizeof(char*));
	    xkb_model.desc = (char**)XtRealloc((XtPointer)xkb_model.desc,
					       (i + 16) * sizeof(char*));
	}
	xkb_model.name[i] = XtNewString(list->models.desc[i].name);
	xkb_model.desc[i] = XtNewString(list->models.desc[i].desc);
    }
    xkb_model.nelem = i;

    for (i = 0; i < list->layouts.num_desc; i++) {
	if (i % 16 == 0) {
	    xkb_layout.name = (char**)XtRealloc((XtPointer)xkb_layout.name,
						(i + 16) * sizeof(char*));
	    xkb_layout.desc = (char**)XtRealloc((XtPointer)xkb_layout.desc,
						(i + 16) * sizeof(char*));
	}
	xkb_layout.name[i] = XtNewString(list->layouts.desc[i].name);
	xkb_layout.desc[i] = XtNewString(list->layouts.desc[i].desc);
    }
    xkb_layout.nelem = i;

    for (i = 0; i < list->variants.num_desc; i++) {
	if (i % 16 == 0) {
	    xkb_variant.name = (char**)XtRealloc((XtPointer)xkb_variant.name,
						 (i + 16) * sizeof(char*));
	    xkb_variant.desc = (char**)XtRealloc((XtPointer)xkb_variant.desc,
						 (i + 16) * sizeof(char*));
	}
	xkb_variant.name[i] = XtNewString(list->variants.desc[i].name);
	xkb_variant.desc[i] = XtNewString(list->variants.desc[i].desc);
    }
    xkb_variant.nelem = i;

    for (i = 0; i < list->options.num_desc; i++) {
	if (i % 16 == 0) {
	    xkb_option.name = (char**)XtRealloc((XtPointer)xkb_option.name,
						(i + 16) * sizeof(char*));
	    xkb_option.desc = (char**)XtRealloc((XtPointer)xkb_option.desc,
						(i + 16) * sizeof(char*));
	}
	xkb_option.name[i] = XtNewString(list->options.desc[i].name);
	xkb_option.desc[i] = XtNewString(list->options.desc[i].desc);
    }
    xkb_option.nelem = i;

    XkbRF_Free(list, True);

    /* Load configuration */
    XmuSnprintf(name, sizeof(name), "%s%s", XkbConfigDir, XkbConfigFile);
    file = fopen(name, "r");
/*    if ((file = fopen(name, "r")) == NULL) {
	strcpy(name, XkbConfigFile);
	file = fopen(name, "r");
    }*/
    if (file != NULL) {
	if (XkbCFParse(file, XkbCFDflts, xkb_info->xkb, &xkb_info->config) == 0) {
	    fprintf(stderr, "Error parsing config file: ");
	    XkbCFReportError(stderr, name, xkb_info->config.error,
			     xkb_info->config.line);
	}
	fclose(file);
    }

    /* XXX Assumes the first keyboard is the core keyboard */
    while (keyboard != NULL) {
	if (strcasecmp(keyboard->inp_driver, "keyboard") == 0)
	    break;
	keyboard = (XF86ConfInputPtr)(keyboard->list.next);
    }
    if (keyboard == NULL)
	return;

    if (xkb_info->config.model != NULL)
	xkb_info->defs.model = xkb_info->config.model;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbModel"))
	!= NULL)
	xkb_info->defs.model = option->opt_val;
    else
	xkb_info->defs.model = xkb_model.name[0];

    if (xkb_info->config.layout != NULL)
	xkb_info->defs.layout = xkb_info->config.layout;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbLayout"))
	!= NULL)
	xkb_info->defs.layout = option->opt_val;
    else
	xkb_info->defs.layout = xkb_layout.name[0];
}

static xf86ConfigSymTabRec ax_controls[] =
{
    {XkbRepeatKeysMask,	     "RepeatKeys"},
    {XkbSlowKeysMask,	     "SlowKeys"},
    {XkbBounceKeysMask,	     "BounceKeys"},
    {XkbStickyKeysMask,	     "StickyKeys"},
    {XkbMouseKeysMask,	     "MouseKeys"},
    {XkbMouseKeysAccelMask,  "MouseKeysAccel"},
    {XkbAccessXKeysMask,     "AccessxKeys"},
    {XkbAccessXTimeoutMask,  "AccessxTimeout"},
    {XkbAccessXFeedbackMask, "AccessxFeedback"},
    {XkbAudibleBellMask,     "AudibleBell"},
    {XkbOverlay1Mask,	     "Overlay1"},
    {XkbOverlay2Mask,	     "Overlay2"},
    {XkbIgnoreGroupLockMask, "IgnoreGroupLock"},
    {-1,		     ""},
};

static xf86ConfigSymTabRec ax_feedback[] =
{
    {XkbAX_SKPressFBMask,    "SlowKeysPress"},
    {XkbAX_SKAcceptFBMask,   "SlowKeysAccept"},
    {XkbAX_FeatureFBMask,    "Feature"},
    {XkbAX_SlowWarnFBMask,   "SlowWarn"},
    {XkbAX_IndicatorFBMask,  "Indicator"},
    {XkbAX_StickyKeysFBMask, "StickyKeys"},
    {XkbAX_TwoKeysMask,	     "TwoKeys"},
    {XkbAX_LatchToLockMask,  "LatchToLock"},
    {XkbAX_SKReleaseFBMask,  "SlowKeysRelease"},
    {XkbAX_SKRejectFBMask,   "SlowkeysReject"},
    {XkbAX_BKRejectFBMask,   "BounceKeysReject"},
    {XkbAX_DumbBellFBMask,   "DumbBell"},
    {-1,		     ""},
};

Bool
WriteXKBConfiguration(char *filename, XkbConfigRtrnPtr conf)
{
    FILE *fp;
    int i, count;

    if (filename == NULL || conf == NULL ||
	(fp = fopen(filename, "w")) == NULL)
	return (False);

    if (conf->rules_file != NULL)
	fprintf(fp, "Rules			 =	%s\n",
		conf->rules_file);
    if (conf->model != NULL)
	fprintf(fp, "Model			 =	\"%s\"\n",
		conf->model);
    if (conf->layout != NULL)
	fprintf(fp, "Layout			 =	\"%s\"\n",
		conf->layout);
    if (conf->variant != NULL)
	fprintf(fp, "Variant			 =	%s\n",
		conf->variant);
    if (conf->options != NULL)
	fprintf(fp, "Options			 =	%s\n",
		conf->options);
    if (conf->keymap != NULL)
	fprintf(fp, "Keymap			 =	%s\n",
		conf->keymap);
    if (conf->keycodes != NULL)
	fprintf(fp, "Keycodes		 =	%s\n",
		conf->keycodes);
    if (conf->geometry != NULL)
	fprintf(fp, "Geometry		 =	%s\n",
		conf->geometry);
    if (conf->phys_symbols != NULL)
	fprintf(fp, "RealSymbols		 =	%s\n",
		conf->phys_symbols);
    if (conf->symbols != NULL)
	fprintf(fp, "Symbols			 =	%s\n",
		conf->symbols);
    if (conf->types != NULL)
	fprintf(fp, "Types			 =	%s\n",
		conf->types);
    if (conf->compat != NULL)
	fprintf(fp, "Compat			 =	%s\n",
		conf->compat);

    if (conf->click_volume > 0)
	fprintf(fp, "ClickVolume		 =	%d\n",
		conf->click_volume);
    if (conf->bell_volume > 0)
	fprintf(fp, "BellVolume		 =	%d\n",
		conf->bell_volume);
    if (conf->bell_pitch > 0)
	fprintf(fp, "BellPitch		 =	%d\n",
		conf->bell_pitch);
    if (conf->bell_duration > 0)
	fprintf(fp, "BellDuration		 =	%d\n",
		conf->bell_duration);

    if (conf->repeat_delay > 0)
	fprintf(fp, "RepeatDelay		 =	%d\n",
		conf->repeat_delay);
    if (conf->repeat_interval > 0)
	fprintf(fp, "RepeatInterval		 =	%d\n",
		conf->repeat_interval);

    if (conf->slow_keys_delay > 0)
	fprintf(fp, "SlowKeysDelay		 =	%d\n",
		conf->slow_keys_delay);

    if (conf->debounce_delay > 0)
	fprintf(fp, "DebounceDelay		 =	%d\n",
		conf->debounce_delay);

    if (conf->mk_delay > 0)
	fprintf(fp, "MouseKeysDelay		 =	%d\n",
		conf->mk_delay);
    if (conf->mk_interval > 0)
	fprintf(fp, "MouseKeysInterval	 =	%d\n",
		conf->mk_interval);
    if (conf->mk_time_to_max > 0)
	fprintf(fp, "MouseKeysTimeToMax	 =	%d\n",
		conf->mk_time_to_max);
    if (conf->mk_max_speed > 0)
	fprintf(fp, "MouseKeysMaxSpeed	 =	%d\n",
		conf->mk_max_speed);
    fprintf(fp, "MouseKeysCurve		 =	%d\n", conf->mk_curve);

    fprintf(fp, "AccessXTimeout		 =	%d\n", conf->ax_timeout);
    if (conf->initial_ctrls != 0) {
	fprintf(fp, "Controls		%c=	",
		conf->replace_initial_ctrls ? ' ' : '+');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->initial_ctrls & ax_controls[i].token)
		== ax_controls[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_controls[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_ctrls_on != 0) {
	fprintf(fp, "AcessXTimeoutCtrlsOn	%c=	",
		conf->replace_axt_ctrls_on ? ' ' : '+');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->axt_ctrls_on & ax_controls[i].token)
		== ax_controls[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_controls[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_ctrls_off != 0) {
	fprintf(fp, "AcessXTimeoutCtrlsOff	%c=	",
		conf->replace_axt_ctrls_off ? ' ' : '-');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->axt_ctrls_off & ax_controls[i].token)
		== ax_controls[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_controls[i].name);
	fprintf(fp, "\n");
    }

    if (conf->initial_opts != 0) {
	fprintf(fp, "Feedback		%c=	",
		conf->replace_initial_opts ? ' ' : '+');
	for (i = count = 0; *ax_feedback[i].name; i++)
	    if ((conf->initial_opts & ax_feedback[i].token)
		== ax_feedback[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_feedback[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_opts_on != 0) {
	fprintf(fp, "AcessXTimeoutFeedbackOn	%c=	",
		conf->replace_axt_opts_on ? ' ' : '+');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->axt_opts_on & ax_feedback[i].token)
		== ax_feedback[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_feedback[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_opts_off != 0) {
	fprintf(fp, "AcessXTimeoutFeedbackOff%c=	",
		conf->replace_axt_opts_off ? ' ' : '-');
	for (i = count = 0; *ax_feedback[i].name; i++)
	    if ((conf->axt_opts_off & ax_feedback[i].token)
		== ax_feedback[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_feedback[i].name);
	fprintf(fp, "\n");
    }

    fclose(fp);

    return (True);
}

void
UpdateKeyboard(Bool load)
{
    static XkbRF_RulesPtr rules;
    XkbComponentNamesRec comps;
    XkbDescPtr xkb;

    if (rules == NULL) {
	FILE *fp;

	if ((fp = fopen(XkbRulesFile, "r")) == NULL) {
	    fprintf(stderr, "Can't open rules file\n");
	    exit(1);
	}

	if ((rules = XkbRF_Create(0, 0)) == NULL) {
	    fclose(fp);
	    fprintf(stderr, "Can't create rules structure\n");
	    exit(1);
	}

	if (!XkbRF_LoadRules(fp, rules)) {
	    fclose(fp);
	    XkbRF_Free(rules, True);
	    fprintf(stderr, "Can't load rules\n");
	    exit(1);
	}
	fclose(fp);
    }

    bzero((char*)&comps, sizeof(XkbComponentNamesRec));
    XkbRF_GetComponents(rules, &(xkb_info->defs), &comps);

    xkb = XkbGetKeyboardByName(DPY, XkbUseCoreKbd, &comps,
			       XkbGBN_AllComponentsMask, 0, load);

    if (xkb == NULL || xkb->geom == NULL) {
	fprintf(stderr, "Couldn't get keyboard\n");
	exit(1);
    }
    if (xkb->names->geometry == 0)
	xkb->names->geometry = xkb->geom->name;

    XkbFreeKeyboard(xkb_info->xkb, 0, False);

    xkb_info->xkb = xkb;

    XtFree(comps.keymap);
    XtFree(comps.keycodes);
    XtFree(comps.compat);
    XtFree(comps.types);
    XtFree(comps.symbols);
    XtFree(comps.geometry);

    if (kbd != NULL)
	XClearArea(XtDisplay(configp), XtWindow(kbd), 0, 0, 0, 0, True);
}

static void
KeyboardModelCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    int i;

    for (i = 0; i < xkb_model.nelem; i++)
	if (strcmp(XtName(w), xkb_model.name[i]) == 0)
	    break;
    XtSetArg(args[0], XtNlabel, xkb_model.desc[i]);
    XtSetValues(modelb, args, 1);
    model = xkb_info->defs.model = xkb_model.name[i];
    UpdateKeyboard(False);
}

static void
KeyboardLayoutCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    int i;

    for (i = 0; i < xkb_layout.nelem; i++)
	if (strcmp(XtName(w), xkb_layout.name[i]) == 0)
	    break;
    XtSetArg(args[0], XtNlabel, xkb_layout.desc[i]);
    XtSetValues(layoutb, args, 1);
    layout = xkb_info->defs.layout = xkb_layout.name[i];
}

/*ARGSUSED*/
static void
KeyboardApplyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    UpdateKeyboard(True);
}

void
KeyboardModelAndLayout(XF86SetupInfo *info)
{
    static int first = 1;
    static Widget kbdml;
    Arg args[1];
    int i;

    if (first) {
	Widget label, popup, sme;

	first = 0;

	kbdml = XtCreateWidget("keyboardML", formWidgetClass,
			       configp, NULL, 0);

	/* MODEL */
	label = XtCreateManagedWidget("labelM", labelWidgetClass,
				      kbdml, NULL, 0);
	modelb = XtVaCreateManagedWidget("model", menuButtonWidgetClass, kbdml,
					 XtNmenuName, "modelP",
					 NULL, 0);
	popup = XtCreatePopupShell("modelP", simpleMenuWidgetClass,
				   modelb, NULL, 0);
	for (i = 0; i < xkb_model.nelem; i++) {
	    sme = XtVaCreateManagedWidget(xkb_model.name[i], smeBSBObjectClass,
					  popup,
					  XtNlabel, xkb_model.desc[i],
					  NULL, 0);
	    XtAddCallback(sme, XtNcallback,  KeyboardModelCallback, NULL);
	}

	/* LAYOUT */
	label = XtCreateManagedWidget("labelL", labelWidgetClass,
				      kbdml, NULL, 0);
	layoutb = XtVaCreateManagedWidget("layout", menuButtonWidgetClass, kbdml,
					  XtNmenuName, "layoutP",
					  XtNlabel, xkb_layout.desc[0],
					  NULL, 0);
	popup = XtCreatePopupShell("layoutP", simpleMenuWidgetClass,
				   layoutb, NULL, 0);
	for (i = 0; i < xkb_layout.nelem; i++) {
	    sme = XtVaCreateManagedWidget(xkb_layout.name[i], smeBSBObjectClass,
					  popup,
					  XtNlabel, xkb_layout.desc[i],
					  NULL, 0);
	    XtAddCallback(sme, XtNcallback,  KeyboardLayoutCallback, NULL);
	}

	kbd = XtCreateManagedWidget("keyboard", coreWidgetClass,
				    kbdml, NULL, 0);

	apply = XtCreateManagedWidget("apply", commandWidgetClass,
				      kbdml, NULL, 0);
	XtAddCallback(apply, XtNcallback, KeyboardApplyCallback, NULL);

	XtRealizeWidget(kbdml);

	XtAddEventHandler(kbd, ExposureMask, False, XkbUIEventHandler, NULL);
	/* Force the first update */
	XClearArea(XtDisplay(kbd), XtWindow(kbd), 0, 0, 0, 0, True);
    }

    for (i = 0; i < xkb_model.nelem; i++)
	if (strcmp(model, xkb_model.name[i]) == 0) {
	    XtSetArg(args[0], XtNlabel, xkb_model.desc[i]);
	    XtSetValues(modelb, args, 1);
	    break;
	}

    for (i = 0; i < xkb_layout.nelem; i++)
	if (strcmp(layout, xkb_layout.name[i]) == 0) {
	    XtSetArg(args[0], XtNlabel, xkb_layout.desc[i]);
	    XtSetValues(layoutb, args, 1);
	    break;
	}

    XtChangeManagedSet(&current, 1, NULL, NULL, &kbdml, 1);
    current = kbdml;
}
