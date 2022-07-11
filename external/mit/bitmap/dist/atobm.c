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
 * atobm - ascii to bitmap filter
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include <X11/Xfuncproto.h>
#include <stdlib.h>

static char *ProgramName;

static void doit(FILE *fp, const char *filename, const char *chars,
		 int xhot, int yhot, const char *name);

static void _X_NORETURN _X_COLD
usage (const char *msg)
{
    if (msg)
	fprintf(stderr, "%s: %s\n", ProgramName, msg);
    fprintf (stderr, "usage:  %s [-options ...] [filename]\n\n%s\n",
	     ProgramName,
             "where options include:\n"
             "    -chars cc        chars to use for 0 and 1 bits, respectively\n"
             "    -name variable   name to use in bitmap file\n"
             "    -xhot number     x position of hotspot\n"
             "    -yhot number     y position of hotspot\n");
    exit (1);
}

static void _X_NORETURN _X_COLD
missing_arg (const char *option)
{
    char msg[32];

    snprintf(msg, sizeof(msg), "%s requires an argument", option);
    usage(msg);
}

static char *
cify_name (char *name)
{
    int length = name ? strlen (name) : 0;
    int i;

    for (i = 0; i < length; i++) {	/* strncpy (result, begin, length); */
	char c = name[i];
	if (!((isascii(c) && isalnum(c)) || c == '_')) name[i] = '_';
    }
    return name;
}

static char *
StripName(char *name)
{
  char *begin = strrchr(name, '/');
  char *end, *result;
  int length;

  begin = (begin ? begin+1 : name);
  end = strchr(begin, '.');	/* change to strrchr to allow longer names */
  length = (end ? (end - begin) : strlen (begin));
  result = (char *) malloc (length + 1);
  strncpy (result, begin, length);
  result [length] = '\0';
  return (result);
}

int
main (int argc, char *argv[])
{
    int i;
    int xhot = -1, yhot = -1;
    char *filename = NULL;
    const char *chars = "-#";
    const char *name = NULL;
    FILE *fp;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case '\0':
		filename = NULL;
		continue;
	      case 'c':
		if (++i >= argc) missing_arg("-chars");
		chars = argv[i];
		continue;
	      case 'n':
		if (++i >= argc) missing_arg("-name");
		name = argv[i];
		continue;
	      case 'x':
		if (++i >= argc) missing_arg("-xhot");
		xhot = atoi (argv[i]);
		continue;
	      case 'y':
		if (++i >= argc) missing_arg("-yhot");
		yhot = atoi (argv[i]);
		continue;
	      default:
		fprintf(stderr, "%s: unrecognized option '%s'\n",
			ProgramName, argv[i]);
		usage (NULL);
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

    if (filename) {
	fp = fopen (filename, "r");
	if (!fp) {
	    fprintf (stderr, "%s:  unable to open file \"%s\".\n",
		     ProgramName, filename);
	    exit (1);
	}
    } else {
	fp = stdin;
    }

    if (!name)
	name = filename ? cify_name (StripName (filename)) : "";
    doit (fp, filename, chars, xhot, yhot, name);

    if (filename) (void) fclose (fp);
    exit (0);
}

struct _scan_list {
    int allocated;
    int used;
    unsigned char *scanlines;
    struct _scan_list *next;
};

#define NTOALLOC 16
static inline struct _scan_list *
_new_scan_list(int bytes_per_scanline) {
    struct _scan_list *slist = (struct _scan_list *) calloc (1, sizeof(*slist));
    if (!slist) {
	return NULL;
    }
    slist->allocated = NTOALLOC * bytes_per_scanline;
    slist->scanlines = (unsigned char *) calloc(slist->allocated, 1);
    if (!slist->scanlines) {
        free(slist);
        return NULL;
    }
    slist->used = 0;
    slist->next = NULL;

    return slist;
}

static void
doit (FILE *fp,
      const char *filename,
      const char *chars,
      int xhot, int yhot,
      const char *name)
{
    int i, j;
    int last_character;
    char buf[BUFSIZ];
    char *cp, *newline;
    int width = 0, height = 0;
    int len;
    int removespace = (((isascii(chars[0]) && isspace(chars[0])) ||
			(isascii(chars[1]) && isspace(chars[1]))) ? 0 : 1);
    int lineno = 0;
    int bytes_per_scanline = 0;
    struct _scan_list *head = NULL, *slist = NULL;
    static unsigned char masktable[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    int padded = 0;

    while (1) {
	buf[0] = '\0';
	lineno++;
	if (fgets (buf, sizeof buf, fp) == NULL) break;

	cp = buf;
	if (removespace) {
	    for (cp = buf; *cp && isascii(*cp) && isspace(*cp); cp++) ;
	}
	if (*cp == '\n' || !*cp) continue;  /* empty line */

	newline = strchr(cp, '\n');
	if (!newline) {
	    fprintf (stderr, "%s:  line %d too long.\n",
		     ProgramName, lineno);
	    return;
	}

	if (removespace) {
	    for (; --newline > cp && isascii(*newline) && isspace(*newline); );
	    newline++;
	}

	if (newline == cp) continue;

	*newline = '\0';
	len = strlen (cp);

	if (head == NULL) {
	    width = len;
	    padded = ((width & 7) != 0);
	    bytes_per_scanline = (len + 7) / 8;
	    head = slist = _new_scan_list(bytes_per_scanline);

            if (!slist) {
                fprintf (stderr, "%s:  unable to allocate scan list\n", ProgramName);
                return;
            }
	} else if (width != len) {
	    fprintf (stderr,
		     "%s:  line %d is %d characters wide instead of %d\n",
		     ProgramName, lineno, len, width);
	    goto bail;
	}

	if (slist->used + 1 >= slist->allocated) {
	    struct _scan_list *old = slist;
	    old->next = slist = _new_scan_list(bytes_per_scanline);

	    if (!slist) {
	        fprintf (stderr, "%s:  unable to allocate scan list\n", ProgramName);
		goto bail;
	    }
	}

	/* okay, parse the line and stick values into the scanline array */
	for (i = 0; i < width; i++) {
	    int ind = (i & 7);
	    int on = 0;

	    if (cp[i] == chars[1]) {
		on = 1;
	    } else if (cp[i] != chars[0]) {
		fprintf (stderr, "%s:  bad character '%c' on line %d\n",
			 ProgramName, cp[i], lineno);
	    }

	    if (on) slist->scanlines[slist->used] |= masktable[ind];
	    if (ind == 7) slist->used++;
	}
	if (padded) slist->used++;
	height++;
    }

    printf ("#define %s_width %d\n", name, width);
    printf ("#define %s_height %d\n", name, height);
    if (xhot >= 0) printf ("#define %s_x_hot %d\n", name, xhot);
    if (yhot >= 0) printf ("#define %s_y_hot %d\n", name, yhot);
    printf ("\n");
    printf ("static unsigned char %s_bits[] = {\n", name);

    j = 0;
    last_character = height * bytes_per_scanline - 1;
    for (slist = head; slist; slist = slist->next) {
	for (i = 0; i < slist->used; i++) {
	    printf (" 0x%02x", slist->scanlines[i]);
	    if (j != last_character) putchar (',');
	    if ((j % 12) == 11) putchar ('\n');
	    j++;
	}
    }
    printf (" };\n");
  bail:
    for (slist = head; slist != NULL; slist = head) {
        head = slist->next;
        free(slist->scanlines);
        free(slist);
    }
    return;
}
