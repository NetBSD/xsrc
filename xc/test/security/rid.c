#include <stdio.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/extensions/security.h>

int total_error_count = 0; /* total # of errors during entire run */
/* A testset is a group of logically related tests, e.g., all the tests
 * for a certain protocol request.
 */
char *testsetname;	/* string name of current testset */
int errs_this_testset;	/* number of errors that have occured this testset */
/* A test tries to validate a specific statement in the spec.  Roughly
 * equivalent to an IC (invocable component) in the TET framework.
 */
char *testname;		/* string name of current test */
int errs_this_test;	/* number of errors that have occured this test */
int total_tests = 0;	/* total number of tests executed */
int expected_protocol_error;
int received_protocol_error;
int security_majorop;		/* opcode for SECURITY */
int security_event, security_error;  /* event and error base for SECURITY */
char *display_env = NULL;	/* -display argument */

/* some utilities for handling errors and demarcating start/end of
 * test modules
 */

/* Call this when an error occurs, passing a string that describes the error.*/

void
report_error(char *err_fmt, ...)
{
    va_list an;

    fprintf(stderr, "Error: %s %s: ", testsetname, testname);
    va_start(an, err_fmt);
    vfprintf(stderr, err_fmt, an);
    va_end(an);
    fprintf(stderr, "\n");
    fflush(stderr);
    errs_this_test++;
}

/* When you're not expecting a protocol error, make sure this is installed
 * via XSetErrorHandler.
 */
int
protocol_error_unexpected(dpy, errevent)
Display *dpy;
XErrorEvent *errevent;
{
    char buf[80];
    XGetErrorText(dpy, errevent->error_code, buf, sizeof (buf));
    report_error(buf);
    return 1;
}

/* When you *are* expecting a protocol error, make sure this is installed
 * via XSetErrorHandler.  You can check the global protocol_error to see
 * if the expected error actually occured.
 */
int
protocol_error_expected(dpy, errevent)
Display *dpy;
XErrorEvent *errevent;
{
    char buf[80];
    XGetErrorText(dpy, errevent->error_code, buf, sizeof (buf));

    if (expected_protocol_error != errevent->error_code)
	report_error("wrong error %d (%s), wanted %d\n",
		     errevent->error_code, buf, expected_protocol_error);
    received_protocol_error = errevent->error_code;
    return 1;
}

void
expect_protocol_error(err)
    int err;
{
    expected_protocol_error = err;
    received_protocol_error = 0;
    XSetErrorHandler(protocol_error_expected);
}

void
verify_error_received()
{
    if (received_protocol_error == 0)
	report_error("error %d never received", expected_protocol_error);
    XSetErrorHandler(protocol_error_unexpected);
}

/* Call this at the start of a test with a short string description of
 * the test.
 */
void
begin_test(char *t)
{
    testname = t;
    errs_this_test = 0;
    XSetErrorHandler(protocol_error_unexpected);
}

/* Call this at the end of a test. */
void end_test()
{
    if (errs_this_test)
    {
	printf("End test %s with %d errors\n\n", testname, errs_this_test);
	fflush(stdout);
    }
    errs_this_testset += errs_this_test;
    total_tests++;
}


/* Call this at the start of a testset with a short string description of the
 * testset.
 */
void
begin_test_set(tn, tc, uc)
char *tn;
Display **tc; /* trusted connection */
Display **uc; /* untrusted connection */
{
    Display *dpy;
    int major_version, minor_version;
    XSecurityAuthorization id_return;
    int status;
    Xauth *auth_in, *auth_return;

    testsetname = tn;
    errs_this_testset = 0;
    printf("Start testset %s\n", testsetname);
    fflush(stdout);
    XSetAuthorization(NULL, 0, NULL, 0); /* use default auth */
    *tc = XOpenDisplay(display_env);
    if (!*tc)
    {
	report_error("Failed to open trusted connection\n");
	exit(1);
    }
    XSynchronize(*tc, True); /* so we get errors at convenient times */
    XSetErrorHandler(protocol_error_unexpected);
    if (!XQueryExtension(*tc, "SECURITY", &security_majorop, &security_event,
			 &security_error))
    {
	report_error("Failed to find SECURITY extension");
	exit(1);
    }

    /* now make untrusted connection */

    status = XSecurityQueryExtension(*tc, &major_version, &minor_version);
    if (!status)
    {
	report_error("%s: couldn't query Security extension");
	exit (1);
    }

    auth_in = XSecurityAllocXauth();
    auth_in->name = "MIT-MAGIC-COOKIE-1";
    auth_in->name_length = strlen(auth_in->name);

    auth_return = XSecurityGenerateAuthorization(*tc, auth_in, 0, NULL,
						 &id_return);
    if (!auth_return)
    {
	report_error("couldn't generate untrusted authorization\n");
	exit (1);
    }

    XSetAuthorization(auth_return->name, auth_return->name_length,
		      auth_return->data, auth_return->data_length);

    *uc = XOpenDisplay(display_env);
    if (!*uc)
    {
	report_error("Failed to open untrusted connection\n");
	exit(1);
    }
    XSynchronize(*uc, True); /* so we get errors at convenient times */
    XSecurityFreeXauth(auth_in);
    XSecurityFreeXauth(auth_return);
}

/* Call this at the end of a testset. */
void
end_test_set(tc, uc)
Display *tc, *uc;
{
    printf("End testset %s with %d errors\n\n", testsetname, errs_this_testset);
    fflush(stdout);
    total_error_count += errs_this_testset;
    XCloseDisplay(tc);
    XCloseDisplay(uc);
}

void
window_resource_tests()
{
    Window tw, uw, root;
    Display *tc, *uc;
    XSetWindowAttributes swa;
    int screen;
    Visual *visual;
    int depth;
    XImage *image;
    XID id;
    GC gc, tgc, ugc;
    Colormap *pcm;
    int num;
    Atom *pa;

    begin_test_set("Window resource access", &tc, &uc);

    root = DefaultRootWindow(tc);
    screen = DefaultScreen(tc);
    visual = DefaultVisual(tc, screen);
    depth = DefaultDepth(tc, screen);

    swa.override_redirect = True;
    tw = XCreateWindow(tc, root, 0, 0, 10, 10, 1, depth, InputOutput,
		       visual, CWOverrideRedirect, &swa);
    XMapWindow(tc, tw);
    uw = XCreateWindow(uc, root, 20, 20, 10, 10, 1, depth, InputOutput,
			     visual, CWOverrideRedirect, &swa);
    XMapWindow(uc, uw);

    tgc = DefaultGC(tc, screen);
    ugc = DefaultGC(uc, screen);

    begin_test("GetImage");

    image = XGetImage(tc, tw, 0, 0, 10, 10, ~0, ZPixmap);
    image = XGetImage(tc, uw, 0, 0, 10, 10, ~0, ZPixmap);
    image = XGetImage(tc, root, 0, 0, 10, 10, ~0, ZPixmap);

    image = XGetImage(uc, uw, 0, 0, 10, 10, ~0, ZPixmap);
    expect_protocol_error(BadDrawable);
    image = XGetImage(uc, tw, 0, 0, 10, 10, ~0, ZPixmap);
    verify_error_received();
    expect_protocol_error(BadDrawable);
    image = XGetImage(uc, root, 0, 0, 10, 10, ~0, ZPixmap);
    verify_error_received();

    end_test();

    begin_test("CreateColormap");

    id = XCreateColormap(tc, root, visual, AllocNone);
    id = XCreateColormap(tc, tw,   visual, AllocNone);
    id = XCreateColormap(tc, uw,   visual, AllocNone);

    id = XCreateColormap(uc, root, visual, AllocNone);
    id = XCreateColormap(uc, uw,   visual, AllocNone);
    expect_protocol_error(BadWindow);
    id = XCreateColormap(uc, tw, visual, AllocNone);
    verify_error_received();

    end_test();

    begin_test("CreateGC");

    gc = XCreateGC(tc, root, 0, NULL);
    gc = XCreateGC(tc, tw,   0, NULL);
    gc = XCreateGC(tc, uw,   0, NULL);

    gc = XCreateGC(uc, root, 0, NULL);
    gc = XCreateGC(uc, uw,   0, NULL);
    expect_protocol_error(BadDrawable);
    gc = XCreateGC(uc, tw, 0, NULL);
    verify_error_received();

    end_test();

    begin_test("CreatePixmap");

    id = XCreatePixmap(tc, root, 10, 10, depth);
    id = XCreatePixmap(tc, tw,   10, 10, depth);
    id = XCreatePixmap(tc, uw,   10, 10, depth);

    id = XCreatePixmap(uc, root, 10, 10, depth);
    id = XCreatePixmap(uc, uw,   10, 10, depth);
    expect_protocol_error(BadDrawable);
    id = XCreatePixmap(uc, tw, 10, 10, depth);
    verify_error_received();

    end_test();

    begin_test("CreateWindow");

    id = XCreateSimpleWindow(tc, root, 0,0,1,1,1,0,0);
    id = XCreateSimpleWindow(tc, tw,   0,0,1,1,1,0,0);
    id = XCreateSimpleWindow(tc, uw,   0,0,1,1,1,0,0);

    id = XCreateSimpleWindow(uc, root, 0,0,1,1,1,0,0);
    id = XCreateSimpleWindow(uc, uw,   0,0,1,1,1,0,0);
    expect_protocol_error(BadWindow);
    id = XCreateSimpleWindow(uc, tw, 0,0,1,1,1,0,0);
    verify_error_received();

    end_test();

    begin_test("ListInstalledColormaps");

    pcm = XListInstalledColormaps(tc, root, &num);
    pcm = XListInstalledColormaps(tc, tw,   &num);
    pcm = XListInstalledColormaps(tc, uw,   &num);

    expect_protocol_error(BadWindow);
    pcm = XListInstalledColormaps(uc, root, &num);
    verify_error_received();
    pcm = XListInstalledColormaps(uc, uw,   &num);
    expect_protocol_error(BadWindow);
    pcm = XListInstalledColormaps(uc, tw,   &num);
    verify_error_received();

    end_test();

    begin_test("ListProperties");

    pa = XListProperties(tc, root, &num);
    pa = XListProperties(tc, tw,   &num);
    pa = XListProperties(tc, uw,   &num);

    pa = XListProperties(uc, root, &num);
    pa = XListProperties(uc, uw,   &num);
    expect_protocol_error(BadWindow);
    pa = XListProperties(uc, tw,   &num);
    verify_error_received();

    end_test();

    begin_test("DrawPoint");

    XDrawPoint(tc, root, tgc, 1, 1);
    XDrawPoint(tc, tw, tgc, 1, 1);
    XDrawPoint(tc, uw, tgc, 1, 1);

    expect_protocol_error(BadDrawable);
    XDrawPoint(uc, root, ugc, 1, 1);
    verify_error_received();
    XDrawPoint(uc, uw, ugc, 1, 1);
    expect_protocol_error(BadDrawable);
    XDrawPoint(uc, tw, ugc, 1, 1);
    verify_error_received();

    end_test();
    end_test_set(tc, uc);

} /* window_resource_tests */

pixmap_resource_tests()
{
    Window root;
    Pixmap tp, up;
    Display *tc, *uc;
    int screen;
    Visual *visual;
    int depth;
    XImage *image;
    XID id;
    int num;
    GC tgc, ugc;

    begin_test_set("Pixmap resource access", &tc, &uc);

    root = DefaultRootWindow(tc);
    screen = DefaultScreen(tc);
    visual = DefaultVisual(tc, screen);
    depth = DefaultDepth(tc, screen);

    tp = XCreatePixmap(tc, root, 10, 10, depth);
    up = XCreatePixmap(uc, root, 10, 10, depth);

    tgc = DefaultGC(tc, screen);
    ugc = DefaultGC(uc, screen);

    begin_test("DrawPoint");

    XDrawPoint(tc, tp, tgc, 1, 1);
    XDrawPoint(tc, up, tgc, 1, 1);

    XDrawPoint(uc, up, ugc, 1, 1);
    expect_protocol_error(BadDrawable);
    XDrawPoint(uc, tp, ugc, 1, 1);
    verify_error_received();

    end_test();

    end_test_set(tc, uc);

} /* pixmap_resource_tests */

font_resource_tests()
{
    Window root;
    Font tf, uf;
    Display *tc, *uc;
    int screen;
    Visual *visual;
    int depth;
    XImage *image;
    XID id;
    int num;
    XGCValues gcvals;
    GC gc;

    begin_test_set("Font resource access", &tc, &uc);

    root = DefaultRootWindow(tc);
    screen = DefaultScreen(tc);
    visual = DefaultVisual(tc, screen);
    depth = DefaultDepth(tc, screen);

    tf = XLoadFont(tc, "fixed");
    uf = XLoadFont(uc, "fixed");

/* hard to test this because Xlib eats BadFont errors */
#if 0
    begin_test("CreateGC");

    gcvals.font = tf;
    gc = XCreateGC(tc, root, GCFont, &gcvals);
    gcvals.font = uf;
    gc = XCreateGC(tc, root, GCFont, &gcvals);

    expect_protocol_error(BadFont);
    gcvals.font = tf;
    gc = XCreateGC(uc, root, GCFont, &gcvals);
    verify_error_received();
    gcvals.font = uf;
    gc = XCreateGC(uc, root, GCFont, &gcvals);

    end_test();
#endif

    end_test_set(tc, uc);

} /* font_resource_tests */

cursor_resource_tests()
{
    Window root;
    Cursor tcr, ucr;
    Display *tc, *uc;
    int screen;
    Visual *visual;
    int depth;
    XImage *image;
    XID id;
    int num;
    XGCValues gcvals;
    GC gc;
    XColor c;

    begin_test_set("Cursor resource access", &tc, &uc);

    root = DefaultRootWindow(tc);
    screen = DefaultScreen(tc);
    visual = DefaultVisual(tc, screen);
    depth = DefaultDepth(tc, screen);

    tcr = XCreateFontCursor(tc, 0);
    ucr = XCreateFontCursor(uc, 0);

    begin_test("RecolorCursor");

    XRecolorCursor(tc, tcr, &c, &c);
    XRecolorCursor(tc, ucr, &c, &c);
    expect_protocol_error(BadCursor);
    XRecolorCursor(uc, tcr, &c, &c);
    verify_error_received();
    XRecolorCursor(uc, ucr, &c, &c);

    end_test();

    end_test_set(tc, uc);

} /* cursor_resource_tests */

gc_resource_tests()
{
    Window root;
    GC  tgc, ugc;
    Pixmap up;
    Display *tc, *uc;
    int screen;
    Visual *visual;
    int depth;
    XID id;
    int num;
    XGCValues gcvals;

    begin_test_set("GC resource access", &tc, &uc);

    root = DefaultRootWindow(tc);
    screen = DefaultScreen(tc);
    visual = DefaultVisual(tc, screen);
    depth = DefaultDepth(tc, screen);

    tgc = XCreateGC(tc, root, 0, NULL);
    ugc = XCreateGC(uc, root, 0, NULL);
    up = XCreatePixmap(uc, root, 10, 10, depth);

    begin_test("DrawPoint");

    XDrawPoint(tc, up, tgc, 1, 1);
    XDrawPoint(tc, up, ugc, 1, 1);

    expect_protocol_error(BadGC);
    XDrawPoint(uc, up, tgc, 1, 1);
    verify_error_received();
    XDrawPoint(uc, up, ugc, 1, 1);

    end_test();

    end_test_set(tc, uc);

} /* gc_resource_tests() */

colormap_resource_tests()
{
    Window root;
    Colormap  tcm, ucm;
    Display *tc, *uc;
    int screen;
    Visual *visual;
    int depth;
    XColor color;

    begin_test_set("Colormap resource access", &tc, &uc);

    root = DefaultRootWindow(tc);
    screen = DefaultScreen(tc);
    visual = DefaultVisual(tc, screen);
    depth = DefaultDepth(tc, screen);

    tcm = XCreateColormap(tc, root, visual, AllocNone);
    ucm = XCreateColormap(uc, root, visual, AllocNone);

    begin_test("AllocColor");

    color.pixel = color.red = color.green = color.blue = 0;
    XAllocColor(tc, tcm, &color);
    XAllocColor(tc, ucm, &color);
    expect_protocol_error(BadColor);
    XAllocColor(uc, tcm, &color);
    verify_error_received();
    XAllocColor(uc, ucm, &color);

    end_test();

    end_test_set(tc, uc);

} /* colormap_resource_tests */


int main(argc, argv)
    int argc;
    char **argv;
{
    if (argc >= 2  &&  strcmp(argv[1], "-display") == 0)
	display_env = argv[2];

    window_resource_tests();
    pixmap_resource_tests();
    font_resource_tests();
    cursor_resource_tests();
    gc_resource_tests();
    colormap_resource_tests();

    printf("total tests: %d,   total errors: %d,   errors/test: %f\n",
	   total_tests, total_error_count,
	   (float)total_error_count / total_tests);
    return total_error_count;
}
