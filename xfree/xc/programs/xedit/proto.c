/* $XFree86: xc/programs/xedit/proto.c,v 1.4 2001/10/28 03:34:28 tsi Exp $ */

#include "xedit.h"
#include <stdlib.h>
#include <ctype.h>

#define BUFFERFMT	"\"<#0x%lx-BUFFER>\""

/*
 * Types
 */
typedef struct _XeditReqTrans XeditReqTrans;
typedef struct _XeditReqArgs XeditReqArgs;
typedef union _XeditReqArg XeditReqArg;
typedef struct _XeditReqInfo XeditReqInfo;
typedef Bool (*XeditReqFun)(XeditReqInfo*, XeditReqArgs*, char**);

struct _XeditReqTrans {
    char *req;
    XeditReqFun fun;
    char *args_desc;
};

union _XeditReqArg {
    long integer;
    char *string;
    xedit_flist_item *item;
};

struct _XeditReqArgs {
    XeditReqArg *args;
    int num_args;
};

struct _XeditReqInfo {
    Widget text;
    Widget source;
    Widget sink;
};

/*
 * Prototypes
 */
static xedit_flist_item *StringToFlistItem(char*);
static Bool PointMin(XeditReqInfo*, XeditReqArgs*, char**);
static Bool PointMax(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetPoint(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetPoint(XeditReqInfo*, XeditReqArgs*, char**);
static Bool Insert(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetForeground(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetForeground(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetBackground(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetBackground(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetFont(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetFont(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetWrapMode(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetWrapMode(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetAutoFill(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetAutoFill(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetLeftColumn(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetLeftColumn(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetRightColumn(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetRightColumn(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetJustification(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetJustification(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetVertScrollbar(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetVertScrollbar(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetHorizScrollbar(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetHorizScrollbar(XeditReqInfo*, XeditReqArgs*, char**);
static Bool CreateBuffer(XeditReqInfo*, XeditReqArgs*, char**);
static Bool RemoveBuffer(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetCurrentBuffer(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetCurrentBuffer(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetOtherBuffer(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetOtherBuffer(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetBufferName(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetBufferName(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetBufferFileName(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetBufferFileName(XeditReqInfo*, XeditReqArgs*, char**);

/* todo */
#if 0
static Bool GetForegroundPixmap(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetForegroundPixmap(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetBackgroundPixmap(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetBackgroundPixmap(XeditReqInfo*, XeditReqArgs*, char**);
static Bool GetFontSet(XeditReqInfo*, XeditReqArgs*, char**);
static Bool SetFontSet(XeditReqInfo*, XeditReqArgs*, char**);
#endif

/*
 * Initialization
 */
static XeditReqTrans trans[] = {
/* nil:			means nothing, i.e. NULL
 * (b)uffer		buffer representation
 * (i)nteger:		number
 * (s)tring:		any text, enclosed in "'s
 */

    /* input  = string: buffer-name
     * output = string: buffer-identifier */
    {"create-buffer",		CreateBuffer,		"s"},

    /* input  = nil
     * output = string: converter value, one of "true" and "false" */
    {"get-auto-fill",		GetAutoFill},

    /* input  = nil
     * output = string: background color */
    {"get-background",		GetBackground},

    /* input  = string: buffer-identifier
     * output = string: buffer-file-name */
    {"get-buffer-filename",	GetBufferFileName,	"b"},

    /* input  = string: buffer-identifier
     * output = string: buffer-name */
    {"get-buffer-name",		GetBufferName,		"b"},

    /* input  = nil
     * output = string: buffer-identifier */
    {"get-current-buffer",	GetCurrentBuffer},

    /* input  = nil
     * output = string: font name */
    {"get-font",		GetFont},

    /* input  = nil
     * output = string: foreground color */
    {"get-foreground",		GetForeground},

    /* input  = nil
     * output = string: one of "always" and "never" */
    {"get-horiz-scrollbar",	GetHorizScrollbar},

    /* input  = nil
     * output = string: one of "left", "right", "center" and "full" */
    {"get-justification",	GetJustification},

    /* input  = nil
     * output = integer: left colum, used only if AutoFill is true */
    {"get-left-column",		GetLeftColumn},

    /* input  = nil
     * output = string: buffer-identifier */
    {"get-other-buffer",	GetOtherBuffer},

    /* input  = nil
     * output = integer: current cursor position */
    {"get-point",		GetPoint},

    /* input  = nil
     * output = integer: right colum, used only if AutoFill is true */
    {"get-right-column",	GetRightColumn},

    /* input  = nil
     * output = string: one of "always" and "never" */
    {"get-vert-scrollbar",	GetVertScrollbar},

    /* input  = nil
     * output = string: converter value, one of "never", "line" and "word" */
    {"get-wrap-mode",		GetWrapMode},

    /* input  = string: text to be inserted
     * output = nil */
    {"insert",			Insert,			"s"},

    /* input  = nil
     * output = integer: largest visible cursor position */
    {"point-max",		PointMax},

    /* input  = nil
     * output = integer: smallest visible cursor position */
    {"point-min",		PointMin},

    /* input  = string: buffer-identifier
     * output = nil
     */
    {"remove-buffer",		RemoveBuffer,		"b"},

    /* input  = string: converter value, one of "true" and "false"
     * output = nil */
    {"set-auto-fill",		SetAutoFill,		"s"},

    /* input  = string: background color
     * output = nil */
    {"set-background",		SetBackground,		"s"},

    /* input  = string list: buffer-identifier and new buffer-name
     * output = nil */
    {"set-buffer-name",		SetBufferName,		"bs"},

    /* input  = string list: buffer-identifier and new buffer-file-name
     * output = nil */
    {"set-buffer-filename",	SetBufferFileName,	"bs"},

    /* input  = string: buffer-identifier
     * output = nil */
    {"set-current-buffer",	SetCurrentBuffer,	"b"},

    /* input  = string: font name
     * output = nil */
    {"set-font",		SetFont,		"s"},

    /* input  = string: foreground color
     * output = nil */
    {"set-foreground",		SetForeground,		"s"},

    /* input  = string: one of "always" and "never"
     * output = nil */
    {"set-horiz-scrollbar",	SetHorizScrollbar,	"s"},

    /* input  = string: one of "left", "right", "center" and "full"
     * output = nil */
    {"set-justification",	SetJustification,	"s"},

    /* input  = integer: left colum, used only if AutoFill is true
     * output = nil */
    {"set-left-column",		SetLeftColumn,		"i"},

    /* input  = string: buffer-identifier
     * output = nil */
    {"set-other-buffer",	SetOtherBuffer,		"b"},

    /* input  = integer: cursor position
     * output = nil */
    {"set-point",		SetPoint,		"i"},

    /* input  = integer: right colum, used only if AutoFill is true
     * output = nil */
    {"set-right-column",	SetRightColumn,		"i"},

    /* input  = string: one of "always" and "never"
     * output = nil */
    {"set-vert-scrollbar",	SetVertScrollbar,	"s"},

    /* input  = string: converter value, one of "never", "line" and "word"
     * output = nil */
    {"set-wrap-mode",		SetWrapMode,		"s"},
};

static char *TooFewArguments = "%s: too few arguments, near %s";
static char *TooManyArguments = "%s: too many arguments, near %s";
static char *ExpectingInteger = "%s: expecting integer, near %s";
static char *ExpectingString = "%s: expecting string, near %s";
static char *ErrorParsingString = "%s: error parsing string, near %s";
static char *BadBuffer = "%s: bad buffer spec or buffer does not exist, %s";
static char buffer[512];

/*
 * Implementation
 */
static int
compar(_Xconst void *left, _Xconst void *right)
{
    return (strcmp((char*)left, ((XeditReqTrans*)right)->req));
}

Bool
XeditProto(char *input, char **result)
{
#define FMT(s)		fmt = s; goto proto_error
    XeditReqTrans *req;
    XeditReqArgs args;
    XeditReqInfo info;
    char fun[64], *ptr, *str, *desc, *fmt;
    int i, len;

    *result = "NIL";

    /* get function name */
    ptr = input;
    while (*ptr && isspace(*ptr))	/* whitespaces */
	++ptr;
    str = ptr;
    while (*ptr && !isspace(*ptr))	/* function name */
	++ptr;
    len = ptr - str;
    if (len > sizeof(fun) - 1)
	len = sizeof(fun) - 1;
    strncpy(fun, str, len);
    fun[len] = '\0';

    /* skip whitespaces */
    while (*ptr && isspace(*ptr))
	++ptr;

    req = bsearch(fun, trans, sizeof(trans) / sizeof(trans[0]),
		  sizeof(XeditReqTrans), compar);

    if (req) {
	char *val;
	Bool retval;

	/* parse arguments */
	args.args = NULL;
	args.num_args = 0;

	desc = req->args_desc;
	if (desc == NULL && *ptr) {
	    FMT(TooManyArguments);
	}

	while (1) {
	    /* find next argument */
	    while (*ptr && isspace(*ptr))
		++ptr;
	    if (*ptr == '\0')
		break;
	    str = ptr;
	    /* string argument */
	    if (*desc == 's') {
		unsigned char c;

		if (*ptr != '"') {
		    FMT(ExpectingString);
		}

		val = NULL;
		len = 0;
		++ptr;
		while (*ptr && *ptr != '"') {
		    if (*ptr == '\0') {
			FMT(ErrorParsingString);
		    }
		    if (*ptr == '\\') {
			++ptr;
			switch (*ptr) {
			    case 'a':
				c = '\a';
				break;
			    case 'f':
				c = '\f';
				break;
			    case 'n':
				c = '\n';
				break;
			    case 'r':
				c = '\r';
				break;
			    case 'x':
				c = 0;
				for (i = 0; i < 2; i++) {
				    int dig = -1;

				    if (isdigit(*ptr))
					dig = *ptr - '0';
				    else if (*ptr >= 'a' && *ptr <= 'f')
					dig = *ptr - 'a' + 10;
				    else if (*ptr >= 'A' && *ptr <= 'F')
					dig = *ptr - 'A' + 10;
				    if (dig < 0) {
					FMT(ErrorParsingString);
				    }
				    c = c * 16 + dig;
				}
				break;
			    case '0': case '1': case '2': case '3': case '4':
			    case '5': case '6': case '7': case '8': case '9':
				c = *ptr - '0';
				for (i = 0; i < 2; i++) {
				    ++ptr;
				    if (!isdigit(*ptr) || *ptr > '7') {
					FMT(ErrorParsingString);
				    }
				    c = c * 8 + (*ptr - '0');
				}
				break;
			    case '\0':
				FMT(ErrorParsingString);
			    default:
				c = *ptr;
				break;
			}
		    }
		    else
			c = *ptr;
		    if ((len % 16) == 0)
			val = XtRealloc(val, len + 16);
		    val[len++] = c;
		    ++ptr;
		}
		++ptr;
		val[len] = '\0';
		args.args = (XeditReqArg*)
		    XtRealloc((XtPointer)args.args, sizeof(XeditReqArg) *
			      (args.num_args + 1));
		args.args[args.num_args++].string = val;
	    }
	    else if (*desc == 'i') {
		char *end;
		double number;

		while (*ptr && !isspace(*ptr))
		    ++ptr;
		number = strtod(str, &end);
		if (end != ptr || (long)number != number) {
		    FMT(ExpectingInteger);
		}
		args.args = (XeditReqArg*)
		    XtRealloc((XtPointer)args.args, sizeof(XeditReqArg) *
			      (args.num_args + 1));
		args.args[args.num_args++].integer = (long)number;
	    }
	    else if (*desc == 'b') {
		xedit_flist_item *item;

		while (*ptr && !isspace(*ptr))
		    ++ptr;
		len = ptr - str;
		strncpy(buffer, ptr, sizeof(buffer));
		buffer[len] = '\0';

		item = StringToFlistItem(buffer);
		if (item == NULL) {
		    FMT(BadBuffer);
		}
		args.args = (XeditReqArg*)
		    XtRealloc((XtPointer)args.args, sizeof(XeditReqArg) *
			      (args.num_args + 1));
		args.args[args.num_args++].item = item;
	    }
	    else if (*desc == '\0') {
		FMT(TooManyArguments);
	    }
	    ++desc;
	}

	if (desc && *desc) {
	    FMT(TooFewArguments);
	}

	info.text = textwindow;
	info.source = XawTextGetSource(textwindow);
	info.sink = XawTextGetSink(textwindow);
	retval = (req->fun)(&info, &args, result);
	if (req->args_desc) {
	    for (ptr = req->args_desc, i = 0; *ptr; ptr++, i++) {
		if (*ptr == 's')
		    XtFree(args.args[i].string);
	    }
	    XtFree((XtPointer)args.args);
	}

	return (retval);

proto_error:
	XmuSnprintf(buffer, sizeof(buffer), fmt, fun, ptr);
	XeditPrintf(buffer);
	*result = input;
	if (req->args_desc) {
	    for (ptr = req->args_desc, i = 0; *ptr; ptr++, i++) {
		if (*ptr == 's')
		    XtFree(args.args[i].string);
	    }
	    XtFree((XtPointer)args.args);
	}
	return (False);
    }

    XmuSnprintf(buffer, sizeof(buffer), "unknown request %s", fun);
    XeditPrintf(buffer);
    *result = input;

    return (False);
}

static xedit_flist_item *
StringToFlistItem(char *string)
{
    int i;
    xedit_flist_item *item;

    if (sscanf(string, BUFFERFMT, (long*)(&item)) != 1)
	return (NULL);

    for (i = 0; i < flist.num_itens; i++)
	if (flist.itens[i] == item)
	    return (item);

    return (NULL);
}

/*ARGSUSED*/
static Bool
PointMin(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextPosition point = XawTextSourceScan(info->source, 0,
					      XawstAll, XawsdLeft, 1, True);

    XmuSnprintf(buffer, sizeof(buffer), "%ld", point);
    *result = buffer;

    return (True);
}

/*ARGSUSED*/
static Bool
PointMax(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextPosition point = XawTextSourceScan(info->source, 0,
					      XawstAll, XawsdRight, 1, True);

    XmuSnprintf(buffer, sizeof(buffer), "%ld", point);
    *result = buffer;

    return (True);
}

/*ARGSUSED*/
static Bool
GetPoint(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextPosition point = XawTextGetInsertionPoint(info->text);

    XmuSnprintf(buffer, sizeof(buffer), "%ld", point);
    *result = buffer;

    return (True);
}

static Bool
SetPoint(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextPosition point = args->args[0].integer;

    XawTextSetInsertionPoint(info->text, point);

    return (True);
}

static Bool
Insert(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextPosition point = XawTextGetInsertionPoint(info->text);
    XawTextBlock block;

    block.firstPos = 0;
    block.format = FMT8BIT;
    block.length = strlen(args->args[0].string);
    block.ptr = args->args[0].string;
    XawTextReplace(info->text, point, point, &block);
    XawTextSetInsertionPoint(info->text, point + block.length);

    return (True);
}

/*ARGSUSED*/
static Bool
GetForeground(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    Pixel pixel;
    Arg arg[1];
    XrmValue from, to;

    from.size = sizeof(pixel);
    from.addr = (XtPointer)&pixel;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNforeground, &pixel);
    XtGetValues(info->sink, arg, 1);
    XtConvertAndStore(info->sink, XtRPixel, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetForeground(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    Pixel pixel;
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(pixel);
    to.addr = (XtPointer)&pixel;

    XtConvertAndStore(info->sink, XtRString, &from, XtRPixel, &to);
    XtSetArg(arg[0], XtNforeground, pixel);
    XtSetValues(info->text, arg, 1);

    return (True);
}

/*ARGSUSED*/
static Bool
GetBackground(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    Pixel pixel;
    Arg arg[1];
    XrmValue from, to;

    from.size = sizeof(pixel);
    from.addr = (XtPointer)&pixel;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNbackground, &pixel);
    XtGetValues(info->sink, arg, 1);
    XtConvertAndStore(info->text, XtRPixel, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetBackground(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Pixel pixel;
    Arg arg[1];
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(pixel);
    to.addr = (XtPointer)&pixel;

    XtConvertAndStore(info->sink, XtRString, &from, XtRPixel, &to);
    XtSetArg(arg[0], XtNbackground, pixel);
    XtSetValues(info->text, arg, 1);

    return (True);
}

/*ARGSUSED*/
static Bool
GetFont(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    XFontStruct *font;
    Arg arg[1];
    XrmValue from, to;

    from.size = sizeof(font);
    from.addr = (XtPointer)&font;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNfont, &font);
    XtGetValues(info->sink, arg, 1);
    XtConvertAndStore(info->text, XtRFontStruct, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetFont(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XFontStruct *font;
    Arg arg[1];
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(font);
    to.addr = (XtPointer)&font;

    XtConvertAndStore(info->sink, XtRString, &from, XtRFontStruct, &to);
    XtSetArg(arg[0], XtNfont, font);
    XtSetValues(info->text, arg, 1);

    return (True);
}

/*ARGSUSED*/
static Bool
GetWrapMode(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    XawTextWrapMode wrap;
    Arg arg[1];
    XrmValue from, to;

    from.size = sizeof(wrap);
    from.addr = (XtPointer)&wrap;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNwrap, &wrap);
    XtGetValues(info->text, arg, 1);
    XtConvertAndStore(info->text, XtRWrapMode, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetWrapMode(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextWrapMode wrap;
    Arg arg[1];
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(wrap);
    to.addr = (XtPointer)&wrap;

    XtConvertAndStore(info->text, XtRString, &from, XtRWrapMode, &to);
    XtSetArg(arg[0], XtNwrap, wrap);
    XtSetValues(info->text, arg, 1);

    return (True);
}

/*ARGSUSED*/
static Bool
GetAutoFill(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    Boolean fill;
    Arg arg[1];
    XrmValue from, to;

    from.size = sizeof(fill);
    from.addr = (XtPointer)&fill;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNautoFill, &fill);
    XtGetValues(info->text, arg, 1);
    XtConvertAndStore(info->text, XtRBoolean, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetAutoFill(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Boolean fill;
    Arg arg[1];
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(fill);
    to.addr = (XtPointer)&fill;

    XtConvertAndStore(info->text, XtRString, &from, XtRBoolean, &to);
    XtSetArg(arg[0], XtNautoFill, fill);
    XtSetValues(info->text, arg, 1);

    return (True);
}

/*ARGSUSED*/
static Bool
GetLeftColumn(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    short left;

    XtSetArg(arg[0], XtNleftColumn, &left);
    XtGetValues(info->text, arg, 1);
    XmuSnprintf(buffer, sizeof(buffer), "%d", left);
    *result = buffer;

    return (True);
}

static Bool
SetLeftColumn(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    int left;

    left = args->args[0].integer;
    XtSetArg(arg[0], XtNleftColumn, left);
    XtSetValues(info->text, arg, 1);

    return (True);
}

/*ARGSUSED*/
static Bool
GetRightColumn(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    short right;

    XtSetArg(arg[0], XtNrightColumn, &right);
    XtGetValues(info->text, arg, 1);
    XmuSnprintf(buffer, sizeof(buffer), "%d", right);
    *result = buffer;

    return (True);
}

static Bool
SetRightColumn(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    int right;

    right = args->args[0].integer;
    XtSetArg(arg[0], XtNrightColumn, right);
    XtSetValues(info->text, arg, 1);

    return (True);
}

static Bool
GetJustification(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    XawTextJustifyMode justify;
    Arg arg[1];
    XrmValue from, to;

    from.size = sizeof(justify);
    from.addr = (XtPointer)&justify;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNjustifyMode, &justify);
    XtGetValues(info->text, arg, 1);
    XtConvertAndStore(info->text, XtRJustifyMode, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetJustification(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XawTextJustifyMode justify;
    Arg arg[1];
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(justify);
    to.addr = (XtPointer)&justify;

    XtConvertAndStore(info->text, XtRString, &from, XtRJustifyMode, &to);
    XtSetArg(arg[0], XtNjustifyMode, justify);
    XtVaSetValues(info->text, arg, 1);

    return (True);
}

static Bool
GetVertScrollbar(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    Arg arg[1];
    XawTextScrollMode scroll;
    XrmValue from, to;

    from.size = sizeof(scroll);
    from.addr = (XtPointer)&scroll;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNscrollVertical, &scroll);
    XtGetValues(info->text, arg, 1);
    XtConvertAndStore(info->text, XtRScrollMode, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetVertScrollbar(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    XawTextScrollMode scroll;
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(scroll);
    to.addr = (XtPointer)&scroll;

    XtConvertAndStore(info->text, XtRString, &from, XtRScrollMode, &to);
    XtSetArg(arg[0], XtNscrollVertical, scroll);
    XtSetValues(info->text, arg, 1);

    return (True);
}

static Bool
GetHorizScrollbar(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    char *str;
    Arg arg[1];
    XawTextScrollMode scroll;
    XrmValue from, to;

    from.size = sizeof(scroll);
    from.addr = (XtPointer)&scroll;
    to.size = 0;
    to.addr = NULL;

    XtSetArg(arg[0], XtNscrollHorizontal, &scroll);
    XtGetValues(info->text, arg, 1);
    XtConvertAndStore(info->text, XtRScrollMode, &from, XtRString, &to);
    str = to.addr;
    XmuSnprintf(buffer, sizeof(buffer), "\"%s\"", str);
    *result = buffer;

    return (True);
}

static Bool
SetHorizScrollbar(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Arg arg[1];
    XawTextScrollMode scroll;
    XrmValue from, to;

    from.size = strlen(args->args[0].string) + 1;
    from.addr = (XtPointer)args->args[0].string;
    to.size = sizeof(scroll);
    to.addr = (XtPointer)&scroll;

    XtConvertAndStore(info->text, XtRString, &from, XtRScrollMode, &to);
    XtSetArg(arg[0], XtNscrollHorizontal, scroll);
    XtSetValues(info->text, arg, 1);

    return (True);
}

static Bool
CreateBuffer(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    Widget source;
    xedit_flist_item *item;

    source =  XtVaCreateWidget("textSource", international ?
			       multiSrcObjectClass : asciiSrcObjectClass,
			       topwindow,
			       XtNeditType, XawtextEdit,
			       NULL, NULL);

    item = AddTextSource(source, args->args[0].string, NULL, 0, 0);
    XmuSnprintf(buffer, sizeof(buffer), BUFFERFMT, item);
    *result = buffer;

    return (True);
}

static Bool
RemoveBuffer(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    KillTextSource(args->args[0].item);

    return (True);
}

static Bool
GetCurrentBuffer(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    xedit_flist_item *item = flist.current;

    if (item == NULL)
	/* this is probably an error */
	return (True);

    XmuSnprintf(buffer, sizeof(buffer), BUFFERFMT, item);
    *result = buffer;

    return (True);
}

static Bool
SetCurrentBuffer(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    SwitchTextSource(args->args[0].item);

    return (True);
}

static Bool
GetOtherBuffer(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    xedit_flist_item *item = flist.other;

    if (item == NULL)
	/* this is not an error */
	return (True);

    XmuSnprintf(buffer, sizeof(buffer), BUFFERFMT, item);
    *result = buffer;
    return (True);
}

static Bool
SetOtherBuffer(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    flist.other = args->args[0].item;

    return (True);
}

static Bool
GetBufferName(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    *result = args->args[0].item->name;

    return (True);
}

static Bool
SetBufferName(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XtFree(args->args[0].item->name);
    args->args[0].item->name = XtNewString(args->args[0].string);

    return (True);
}

static Bool
GetBufferFileName(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    *result = args->args[0].item->filename;

    return (True);
}

/* this probably should not be allowed */
static Bool
SetBufferFileName(XeditReqInfo *info, XeditReqArgs *args, char **result)
{
    XtFree(args->args[0].item->name);
    args->args[0].item->filename = XtNewString(args->args[0].string);

    return (True);
}
