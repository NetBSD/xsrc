/*
 * $XFree86: xc/programs/xftcache/xftcache.c,v 1.1 2001/01/02 02:48:42 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <X11/Xlib.h>
#include <X11/Xft/XftFreetype.h>
#include <stdio.h>

extern char **XftConfigDirs;

int
main (int argc, char **argv)
{
    int		ret = 0;
    XftFontSet	*set;
    char	**dirs;

    if (!XftInit (0))
    {
	fprintf (stderr, "Can't init Xft library\n");
	return -1;
    }
    /*
     * This will scan all of the directories into a single database
     * which is not useful here
     */
    if (!XftInitFtLibrary())
    {
	fprintf (stderr, "Can't init FreeType library\n");
	return -1;
    }
    if (argv[1])
	dirs = argv+1;
    else
	dirs = XftConfigDirs;
    /*
     * Now scan all of the directories into separate databases
     * and write out the results
     */
    while (*dirs)
    {
	set = XftFontSetCreate ();
	if (!set)
	{
	    fprintf (stderr, "Out of memory in \"%s\"\n", *dirs);
	    ret++;
	}
	else
	{
	    if (!XftDirScan (set, *dirs, True))
	    {
		fprintf (stderr, "Can't scan directory \"%s\"\n", *dirs);
		ret++;
	    }
	    else
	    {
		if (!XftDirSave (set, *dirs))
		{
		    fprintf (stderr, "Can't save cache in \"%s\"\n", *dirs);
		    ret++;
		}
	    }
	    XftFontSetDestroy (set);
	}
	++dirs;
    }
    return ret;
}
