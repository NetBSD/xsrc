/* $XConsortium: xgenauth.c /main/2 1996/09/26 12:52:39 dpw $ */
/*
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

#include <stdio.h>			/* for NULL */
#include <X11/Xos.h>			/* for strchr() and string routines */
#include <X11/Xlib.h>
#include <X11/extensions/security.h>

char *ProgramName;

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

static void usage()
{
    fprintf (stderr, "usage:  %s [options]\n", ProgramName);
    fprintf (stderr, "-display displayname\tserver to query\n");
    fprintf (stderr, "-name authname\tauth protocol name\n");
    fprintf (stderr, "-data authdata\tauth protocol data\n");
    fprintf (stderr, "-trust 0|1\ttrusted|untrusted\n");
    fprintf (stderr, "-group n\tapp group\n");
    fprintf (stderr, "-timeout n\tauth timeout (seconds)\n");
    exit(1);
}

int main (argc, argv)
    int argc;
    char *argv[];
{
    Display *dpy;			/* X connection */
    char *displayname = NULL;		/* server to contact */
    int i;
    int major_version, minor_version;
    XSecurityAuthorization id_return;
    int status;
    Xauth *auth_in, *auth_return;
    XSecurityAuthorizationAttributes attributes;
    unsigned long amask = 0;

    ProgramName = argv[0];

    auth_in = XSecurityAllocXauth();
    auth_in->name = "MIT-MAGIC-COOKIE-1";

    for (i = 1; i < argc; i++)
    {
	char *arg = argv[i];

	if (!strcmp("-display", arg))
	{
	    if (++i >= argc) usage ();
	    displayname = argv[i];
	}
	else if (!strcmp("-name", arg))
	{
	    if (++i >= argc) usage ();
	    auth_in->name = argv[i];
	}
	else if (!strcmp("-data", arg))
	{
	    if (++i >= argc) usage ();
	    auth_in->data = argv[i];
	}
	else if (!strcmp("-trust", arg))
	{
	    if (++i >= argc) usage ();
	    attributes.trust_level = atoi(argv[i]);
	    amask |= XSecurityTrustLevel;
	}
	else if (!strcmp("-group", arg))
	{
	    if (++i >= argc) usage ();
	    attributes.group = atoi(argv[i]);
	    amask |= XSecurityGroup;
	}
	else if (!strcmp("-timeout", arg))
	{
	    if (++i >= argc) usage ();
	    attributes.timeout = atoi(argv[i]);
	    amask |= XSecurityTimeout;
	}
	else usage();
    }

    displayname = XDisplayName(displayname);

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\".\n",
		 ProgramName, displayname);
	exit (1);
    }

    status = XSecurityQueryExtension(dpy, &major_version, &minor_version);
    if (!status)
    {
	fprintf (stderr, "%s: couldn't query Security extension on display \"%s\"\n",
		 ProgramName, displayname);
	exit (1);
    }
    else
    {
	fprintf(stderr, "Security version %d.%d on display \"%s\"\n",
	       major_version, minor_version, displayname);
    }

    auth_in->name_length = strlen(auth_in->name);
    if (auth_in->data) auth_in->data_length = strlen(auth_in->data);

    auth_return = XSecurityGenerateAuthorization(dpy, auth_in, amask,
						 &attributes, &id_return);
    if (!auth_return)
    {
	fprintf (stderr, "%s: couldn't generate authorization\n", ProgramName);
	exit (1);
    }

    fprintf(stderr, "id: %d data length: %d\n",
	    id_return, auth_return->data_length);
    printf("add %s %s ", displayname, auth_in->name);
    printhex(auth_return->data, auth_return->data_length);
    printf("\n");

    XSecurityFreeXauth(auth_in);
    XSecurityFreeXauth(auth_return);
    exit (0);
}
