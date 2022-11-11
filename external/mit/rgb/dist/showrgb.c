/*
 *
Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

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
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef USE_RGB_TXT
#include DBM_HEADER
#ifndef NDBM
#define dbm_open(name,flags,mode) (!dbminit(name))
#define dbm_firstkey(db) (firstkey())
#define dbm_fetch(db,key) (fetch(key))
#define dbm_close(db) dbmclose()
#endif
#endif /* USE_RGB_TXT */

#undef NULL
#include <stdio.h>
#include <X11/Xos.h>
#include <stdlib.h>
#ifndef USE_RGB_TXT
#include "rgb.h"			/* off in server/include/ */
#endif
#include <X11/Xfuncs.h>

static char *ProgramName;
static void dumprgb(const char *filename);

int
main (int argc, char *argv[])
{
    const char *dbname = RGB_DB;

    ProgramName = argv[0];
    if (argc == 2)
	dbname = argv[1];

    dumprgb (dbname);
    exit (0);
}

#ifndef USE_RGB_TXT
static void
dumprgb (const char *filename)
{
#ifdef NDBM
    DBM *rgb_dbm;
#else
    int rgb_dbm;
#endif
    datum key;

    rgb_dbm = dbm_open (filename, O_RDONLY, 0);
    if (!rgb_dbm) {
	fprintf (stderr, "%s:  unable to open rgb database \"%s\"\n",
		 ProgramName, filename);
	exit (1);
    }

#ifndef NDBM
#define dbm_nextkey(db) (nextkey(key))	/* need variable called key */
#endif

    for (key = dbm_firstkey(rgb_dbm); key.dptr != NULL;
	 key = dbm_nextkey(rgb_dbm)) {
	datum value;

	value = dbm_fetch(rgb_dbm, key);
	if (value.dptr) {
	    RGB rgb;
	    unsigned short r, g, b;
	    memcpy(&rgb, value.dptr, sizeof rgb);
#define N(x) (((x) >> 8) & 0xff)
	    r = N(rgb.red);
	    g = N(rgb.green);
	    b = N(rgb.blue);
#undef N
	    printf ("%3u %3u %3u\t\t", r, g, b);
	    fwrite (key.dptr, 1, key.dsize, stdout);
	    putchar ('\n');
	} else {
	    fprintf (stderr, "%s:  no value found for key \"", ProgramName);
	    fwrite (key.dptr, 1, key.dsize, stderr);
	    fprintf (stderr, "\"\n");
	}
    }

    dbm_close (rgb_dbm);
}

#else /* USE_RGB_TXT */
static void
dumprgb (const char *filename)
{
    FILE *rgb;
    char *path;
    char line[BUFSIZ];
    char name[BUFSIZ];
    int lineno = 0;
    int red, green, blue;

#ifdef HAVE_ASPRINTF
    if (asprintf(&path, "%s.txt", filename) == -1) {
        perror (ProgramName);
        exit (1);
    }
#else
    path = malloc(strlen(filename) + 5);
    strcpy(path, filename);
    strcat(path, ".txt");
#endif

    if (!(rgb = fopen(path, "r")) && !(rgb = fopen(filename, "r"))) {
	fprintf (stderr, "%s:  unable to open rgb database \"%s\"\n",
		 ProgramName, filename);
	free(path);
	exit (1);
    }

    while(fgets(line, sizeof(line), rgb)) {
	lineno++;
	if (sscanf(line, "%d %d %d %[^\n]\n", &red, &green, &blue, name) == 4) {
	    if (red >= 0 && red <= 0xff &&
		green >= 0 && green <= 0xff &&
		blue >= 0 && blue <= 0xff) {
		printf ("%3u %3u %3u\t\t%s\n", red, green, blue, name);
	    } else {
		fprintf(stderr, "%s:  value for \"%s\" out of range: %s:%d\n",
		        ProgramName, name, path, lineno);
	    }
	} else if (*line && *line != '!') {
	    fprintf(stderr, "%s:  syntax error: %s:%d\n", ProgramName,
		    path, lineno);
	}
    }

    free(path);
    fclose(rgb);
}

#endif /* USE_RGB_TXT */
