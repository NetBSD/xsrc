/*
 * $XConsortium: embedtest.c /main/2 1996/10/10 16:21:17 kaleb $
 *
Copyright (c) 1996 X Consortium, Inc.  All Rights Reserved.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OF
OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/extensions/Xag.h>
#include <X11/extensions/security.h>

static Widget top;

static XAppGroup app_group;

static unsigned int auth_id;

void printhexdigit(d)
    unsigned int d;
{
    if (d > 9) d += 'A' - 10;
    else       d += '0';
    printf("%c", d);
}

void printhex(data, len)
    unsigned char *data;
    int len;
{
    while (len--)
    {
        unsigned int c = *data++;
        printhexdigit(c >> 4);
        printhexdigit(c & 0xf);
    }
}

static 
void SubstructureRedirectHandler (widget, client_data, event, cont)
    Widget widget;
    XtPointer client_data;
    XEvent *event;
    Boolean *cont;
{
    XWindowChanges xwc;
    unsigned int value_mask;

    switch (event->type) {
    case MapRequest:
	(void) printf ("MapRequest Event received: %x\n",
		event->xmaprequest.window);
	XReparentWindow (XtDisplay (widget), 
		event->xmaprequest.window,
		XtWindow (widget), 0, 0);
	XMapWindow (XtDisplay (widget), event->xmaprequest.window);
	break;
    case ConfigureRequest:
	(void) printf ("%s %x\n", "ConfigureRequest Event received",
		event->xconfigurerequest.window);
	(void) printf ("x: %d, y: %d, width: %d height: %d, border_width: %d, sibling: %x, stack_mode: %d, mask %x\n", 
		event->xconfigurerequest.x,
		event->xconfigurerequest.y,
		event->xconfigurerequest.width,
		event->xconfigurerequest.height,
		event->xconfigurerequest.border_width,
		event->xconfigurerequest.above,
		event->xconfigurerequest.detail,
		event->xconfigurerequest.value_mask);
	xwc.x = event->xconfigurerequest.x;
	xwc.y = event->xconfigurerequest.y;
	xwc.width = event->xconfigurerequest.width;
	xwc.height = event->xconfigurerequest.height;
	xwc.border_width = event->xconfigurerequest.border_width;
	xwc.sibling = event->xconfigurerequest.above;
	xwc.stack_mode = event->xconfigurerequest.detail;
	value_mask = (event->xconfigurerequest.value_mask & 
		(CWX|CWY|CWWidth|CWHeight|CWBorderWidth|CWSibling|CWStackMode));
	XConfigureWindow (XtDisplay (widget),
		event->xconfigurerequest.window,
		value_mask,
		&xwc);
	break;
    default:
	(void) printf ("%s %d\n", "Unexpected Event", event->type);
    }
}

static
void QuitCB (w, client_data, call_data)
    Widget w;
    XtPointer client_data;
    XtPointer call_data;
{
    XtAppSetExitFlag (XtWidgetToApplicationContext (w));
}

static
void GenAuthCB (w, client_data, call_data)
    Widget w;
    XtPointer client_data;
    XtPointer call_data;
{
    Xauth* auth_in;
    Xauth* auth_out;
    XSecurityAuthorizationAttributes xsa;
    XSecurityAuthorization auth_id;
    unsigned long xsamask;
    Display* dpy = XtDisplay (w);

    auth_in = XSecurityAllocXauth();
    auth_in->name = "MIT-MAGIC-COOKIE-1";
    auth_in->name_length = strlen (auth_in->name);
    auth_in->data = NULL; auth_in->data_length = 0;

    xsa.timeout = 0;
    xsa.trust_level = XSecurityClientUntrusted;
    xsa.group = app_group;
    xsamask = XSecurityTimeout | XSecurityTrustLevel | XSecurityGroup;
    auth_out = XSecurityGenerateAuthorization (dpy, auth_in, 
			xsamask, &xsa, &auth_id);

    printf("add %s %s ", DisplayString(dpy), auth_in->name);
    printhex(auth_out->data, auth_out->data_length);
    printf("\n");

    XSecurityFreeXauth (auth_in);
    XSecurityFreeXauth (auth_out);
}

int main (argc, argv)
    int argc;
    char** argv;
{
    Widget form, command1, command2, html;
    XtAppContext app_con;
    int agmajor, agminor;
    int secmajor, secminor;
    Display* dpy;

    top = XtVaAppInitialize (&app_con,
		"Myapp",
		NULL, 0,
		&argc, argv,
		NULL,
		(String) XtNgeometry, 
		(XtArgVal) "+200+100", 
		NULL);

    form = XtCreateManagedWidget ("form",
		formWidgetClass, top,
		NULL, 0);

    command1 = XtVaCreateManagedWidget ("quit",
		commandWidgetClass,
		form,
		XtNheight, 30, NULL);

    command2 = XtVaCreateManagedWidget ("genauth",
		commandWidgetClass,
		form,
		XtNfromHoriz, command1, 
		XtNheight, 30, NULL);

    html = XtVaCreateManagedWidget ("html",
		labelWidgetClass,
		form,
		XtNfromVert, command1, 
		XtNwidth, 600, 
		XtNheight, 400, NULL);

    XtAddCallback (command1, XtNcallback, QuitCB, NULL);

    XtAddCallback (command2, XtNcallback, GenAuthCB, NULL);

    XtRealizeWidget (top);

    dpy = XtDisplay (top);

    XagQueryVersion(dpy, &agmajor, &agminor);

    XSecurityQueryExtension (dpy, &secmajor, &secminor);

    XagCreateEmbeddedApplicationGroup (dpy, None, None, 0, 0, &app_group);

    XtAddRawEventHandler (html, SubstructureRedirectMask, False,
		SubstructureRedirectHandler, NULL);

    XtRegisterDrawable (dpy, app_group, html);

    XtAppMainLoop (app_con);

    exit (0);
}

