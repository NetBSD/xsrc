/*

Copyright 1988, 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

/*
 * bmtoa - bitmap to ascii filter
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/Xmu/Drawing.h>

#include <stdlib.h>
#include <unistd.h>
#ifndef HAVE_MKSTEMP
extern char *mktemp();
#endif

static char *ProgramName;

static void print_scanline (unsigned int width, unsigned int height,
			    unsigned const char *data, const char *chars);

static void _X_NORETURN
usage (int exitval)
{
    fprintf (stderr, "usage:  %s [-options ...] [filename]\n\n%s\n",
	     ProgramName,
	"where options include:\n"
	"    -chars cc        chars to use for 0 and 1 bits, respectively\n"
	"    -help            print this usage message\n"
	"    -version         print version information\n"
        );
    exit (exitval);
}

static char *
copy_stdin (void)
{
#ifdef WIN32
    static char tmpfilename[] = "/temp/bmtoa.XXXXXX";
#else
    static char tmpfilename[] = "/tmp/bmtoa.XXXXXX";
#endif
    char buf[BUFSIZ];
    FILE *fp = NULL;
    int nread, nwritten;

#ifndef HAVE_MKSTEMP
    if (mktemp (tmpfilename) != NULL)
	fp = fopen (tmpfilename, "w");
#else
    int fd;
    if ((fd = mkstemp(tmpfilename)) >= 0)
	fp = fdopen(fd, "w");
#endif
    if (fp == NULL) {
	fprintf (stderr,
		 "%s:  unable to generate temporary file for stdin.\n",
		 ProgramName);
	exit (1);
    }
    while (1) {
	buf[0] = '\0';
	nread = fread (buf, 1, sizeof buf, stdin);
	if (nread <= 0) break;
	nwritten = fwrite (buf, 1, nread, fp);
	if (nwritten != nread) {
	    fprintf (stderr,
		     "%s:  error copying stdin to file (%d of %d chars)\n",
		     ProgramName, nwritten, nread);
	    (void) fclose (fp);
	    (void) unlink (tmpfilename);
	    exit (1);
	}
    }
    (void) fclose (fp);
    return tmpfilename;
}

int
main (int argc, char *argv[])
{
    const char *filename = NULL;
    int isstdin = 0;
    const char *chars = "-#";
    int i;
    unsigned int width, height;
    unsigned char *data;
    int x_hot, y_hot;
    int status;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	const char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case '\0':
		filename = NULL;
		continue;
	      case 'c':
		if (++i >= argc) {
		    fprintf(stderr, "%s: -chars requires an argument\n",
			    ProgramName);
		    usage(1);
		}
		chars = argv[i];
		continue;
	      case 'h':
		if (strcmp(arg, "-help") == 0) {
		    usage(0);
		}
		goto unknown;
	      case 'v':
		if (strcmp(arg, "-version") == 0) {
		    puts(PACKAGE_STRING);
		    exit(0);
		}
                goto unknown;
	      default:
	      unknown:
		fprintf(stderr, "%s: unrecognized option '%s'\n",
			ProgramName, argv[i]);
		usage(1);
	    }
	} else {
	    filename = arg;
	}
    }

    if (strlen (chars) != 2) {
	fprintf (stderr,
	 "%s:  bad character list \"%s\", must have exactly 2 characters\n",
		 ProgramName, chars);
	exit (1);
    }

    if (!filename) {
	filename = copy_stdin ();
	isstdin = 1;
    }

    status = XmuReadBitmapDataFromFile (filename, &width, &height, &data,
					&x_hot, &y_hot);
    if (isstdin) (void) unlink (filename);  /* don't need it anymore */
    if (status != BitmapSuccess) {
	fprintf (stderr, "%s:  unable to read bitmap from file \"%s\"\n",
		 ProgramName, isstdin ? "(stdin)" : filename);
	exit (1);
    }

    print_scanline (width, height, data, chars);
    exit (0);
}

static void
print_scanline (unsigned int width,
		unsigned int height,
		unsigned const char *data,
		const char *chars)
{
    unsigned const char *dp = data;
    int row, column;
    static unsigned const char masktable[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    int padded = ((width & 7) != 0);

    for (row = 0; row < height; row++) {
	for (column = 0; column < width; column++) {
	    int i = (column & 7);

	    if (*dp & masktable[i]) {
		putchar (chars[1]);
	    } else {
		putchar (chars[0]);
	    }

	    if (i == 7) dp++;
	}
	putchar ('\n');
	if (padded) dp++;
    }
    return;
}

